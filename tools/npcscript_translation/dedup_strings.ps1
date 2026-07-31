param(
    [string]$InPath = "$PSScriptRoot\npcscript_extract.json",
    [string]$OutPath = "$PSScriptRoot\unique_strings.json",
    [string]$OwnerOutPath = "$PSScriptRoot\unique_owners.json"
)

$scripts = Get-Content -Raw -Path $InPath | ConvertFrom-Json

$uniqueMap = New-Object 'System.Collections.Generic.Dictionary[string,int]'
$uniqueList = New-Object System.Collections.ArrayList
$ownerSet = New-Object 'System.Collections.Generic.Dictionary[string,int]'
$ownerList = New-Object System.Collections.ArrayList

foreach ($sc in $scripts) {
    if ($sc.OwnerID -and -not $ownerSet.ContainsKey($sc.OwnerID)) {
        [void]$ownerSet.Add($sc.OwnerID, $ownerList.Count)
        [void]$ownerList.Add($sc.OwnerID)
    }
    foreach ($sub in $sc.Subjects) {
        if ($sub -ne "" -and -not $uniqueMap.ContainsKey($sub)) {
            [void]$uniqueMap.Add($sub, $uniqueList.Count)
            [void]$uniqueList.Add($sub)
        }
    }
    foreach ($cnt in $sc.Contents) {
        if ($cnt -ne "" -and -not $uniqueMap.ContainsKey($cnt)) {
            [void]$uniqueMap.Add($cnt, $uniqueList.Count)
            [void]$uniqueList.Add($cnt)
        }
    }
}

Write-Output "Scripts: $($scripts.Count)"
Write-Output "Unique dialogue strings: $($uniqueList.Count)"
Write-Output "Unique owner names: $($ownerList.Count)"

$uniqueList | ConvertTo-Json -Depth 2 | Out-File -FilePath $OutPath -Encoding utf8
$ownerList | ConvertTo-Json -Depth 2 | Out-File -FilePath $OwnerOutPath -Encoding utf8
Write-Output "Wrote $OutPath and $OwnerOutPath"
