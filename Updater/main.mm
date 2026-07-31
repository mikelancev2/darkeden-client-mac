// Dark Eden Updater - macOS GUI shell.
//
// A plain AppKit window hosting a WKWebView that loads ui/index.html
// (the visual design - background, logo, nav, server/resolution lists,
// play button - all HTML/CSS, styled to match the real Windows Updater's
// look without hand-drawing native controls for each element). This file
// is just the native shell: window creation, a JS<->native bridge, and
// wiring the "Play" button to update_logic.cpp's RunUpdate() followed by
// launching the client.
//
// Not built as a proper signed .app bundle yet (no Info.plist) - see
// Patch/mac/README.md for the Gatekeeper implications of that.

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#include "update_logic.h"
#include <string>

static const char* PATCH_BASE_URL = "https://darkedenclassic.com/patch2";

static NSString* ExecutableDir()
{
	NSString* path = [[NSBundle mainBundle] executablePath];
	return [path stringByDeletingLastPathComponent];
}

static NSString* StageName(UpdateStage stage)
{
	switch (stage) {
		case UpdateStage::Connecting: return @"Connecting";
		case UpdateStage::ManifestLoaded: return @"ManifestLoaded";
		case UpdateStage::CheckingFile: return @"CheckingFile";
		case UpdateStage::Downloading: return @"Downloading";
		case UpdateStage::FileDone: return @"FileDone";
		case UpdateStage::Complete: return @"Complete";
		case UpdateStage::ServerUnreachable: return @"ServerUnreachable";
		case UpdateStage::Error: return @"Error";
	}
	return @"Unknown";
}

@interface UpdaterAppDelegate : NSObject <NSApplicationDelegate, WKScriptMessageHandler, WKUIDelegate>
@property (strong) NSWindow* window;
@property (strong) WKWebView* webView;
@end

@implementation UpdaterAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
	NSRect frame = NSMakeRect(0, 0, 1000, 650);
	self.window = [[NSWindow alloc]
		initWithContentRect:frame
		styleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable)
		backing:NSBackingStoreBuffered
		defer:NO];
	[self.window setTitle:@"Dark Eden Classic"];
	[self.window center];

	WKWebViewConfiguration* config = [[WKWebViewConfiguration alloc] init];
	WKUserContentController* controller = [[WKUserContentController alloc] init];
	[controller addScriptMessageHandler:self name:@"native"];
	config.userContentController = controller;

	self.webView = [[WKWebView alloc] initWithFrame:frame configuration:config];
	self.webView.UIDelegate = self;
	[self.window setContentView:self.webView];

	NSString* htmlPath = [ExecutableDir() stringByAppendingPathComponent:@"ui/index.html"];
	NSURL* htmlUrl = [NSURL fileURLWithPath:htmlPath];
	[self.webView loadFileURL:htmlUrl allowingReadAccessToURL:[htmlUrl URLByDeletingLastPathComponent]];

	[self.window makeKeyAndOrderFront:nil];
	[NSApp activateIgnoringOtherApps:YES];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)app
{
	return YES;
}

// JS -> native
- (void)userContentController:(WKUserContentController*)controller
		didReceiveScriptMessage:(WKScriptMessage*)message
{
	NSDictionary* body = message.body;
	if (![body isKindOfClass:[NSDictionary class]]) return;
	NSString* action = body[@"action"];

	if ([action isEqualToString:@"openUrl"]) {
		NSURL* url = [NSURL URLWithString:body[@"url"]];
		if (url) [[NSWorkspace sharedWorkspace] openURL:url];
	} else if ([action isEqualToString:@"play"]) {
		[self startUpdateAndLaunch:body[@"mode"]];
	}
}

// Links with target="_blank" (e.g. inside the embedded news iframe) try to
// open a new WKWebView by default; send them to the system browser instead.
- (WKWebView*)webView:(WKWebView*)webView
	createWebViewWithConfiguration:(WKWebViewConfiguration*)configuration
	forNavigationAction:(WKNavigationAction*)navigationAction
	windowFeatures:(WKWindowFeatures*)windowFeatures
{
	if (navigationAction.request.URL) {
		[[NSWorkspace sharedWorkspace] openURL:navigationAction.request.URL];
	}
	return nil;
}

- (void)startUpdateAndLaunch:(NSString*)mode
{
	std::string modeStr = mode ? [mode UTF8String] : "0000000003";
	__weak WKWebView* weakWebView = self.webView;

	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		ProgressCallback cb = [weakWebView](const UpdateProgress& p) {
			NSDictionary* dict = @{
				@"stage": StageName(p.stage),
				@"fileName": [NSString stringWithUTF8String:p.fileName.c_str()],
				@"fileIndex": @(p.fileIndex),
				@"fileCount": @(p.fileCount),
				@"message": [NSString stringWithUTF8String:p.message.c_str()]
			};
			NSData* jsonData = [NSJSONSerialization dataWithJSONObject:dict options:0 error:nil];
			NSString* jsonStr = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
			NSString* js = [NSString stringWithFormat:
				@"window.updaterBridge && window.updaterBridge.onProgress(%@)", jsonStr];
			dispatch_async(dispatch_get_main_queue(), ^{
				[weakWebView evaluateJavaScript:js completionHandler:nil];
			});
		};

		bool ok = RunUpdate(PATCH_BASE_URL, cb);

		dispatch_async(dispatch_get_main_queue(), ^{
			if (!ok) {
				return; // error already reported via progress callback; JS re-enables the Play button
			}
			[weakWebView evaluateJavaScript:@"window.updaterBridge && window.updaterBridge.onLaunching()"
				completionHandler:^(id result, NSError* error) {
					// Briefly let "Launching..." render before replacing this process.
					dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.4 * NSEC_PER_SEC)),
						dispatch_get_main_queue(), ^{
							chdir([ExecutableDir() UTF8String]);
							execl("./DarkEden", "DarkEden", modeStr.c_str(), (char*)NULL);
							// Only reached if exec failed - nothing graceful left to do from here.
						});
				}];
		});
	});
}

@end

int main(int argc, const char* argv[])
{
	@autoreleasepool {
		NSApplication* app = [NSApplication sharedApplication];
		UpdaterAppDelegate* delegate = [[UpdaterAppDelegate alloc] init];
		[app setDelegate:delegate];
		[app setActivationPolicy:NSApplicationActivationPolicyRegular];
		[app run];
	}
	return 0;
}
