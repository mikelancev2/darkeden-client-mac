param(
    [string]$Path = "c:\Users\estac\Desktop\Client\Client\Data\Info\NPCScript.inf",
    [string]$OutPath = "$PSScriptRoot\npcscript_extract.json"
)

$cp949 = [System.Text.Encoding]::GetEncoding(949)

$fs = [System.IO.File]::OpenRead($Path)
$br = New-Object System.IO.BinaryReader($fs)

function Read-MString {
    param($br)
    $len = $br.ReadInt32()
    if ($len -eq 0) { return "" }
    if ($len -lt 0 -or $len -gt 65536) { throw "Bad string length $len at offset $($br.BaseStream.Position)" }
    $bytes = $br.ReadBytes($len)
    return $script:cp949.GetString($bytes)
}

$totalScripts = $br.ReadInt32()

$scripts = New-Object System.Collections.ArrayList
$totalChars = 0
$totalStrings = 0
$uniqueStrings = New-Object 'System.Collections.Generic.HashSet[string]'

$fileLen = $fs.Length
try {
for ($i = 0; $i -lt $totalScripts; $i++) {
    $posBefore = $br.BaseStream.Position
    $scriptId = $br.ReadUInt32()
    $ownerId = Read-MString $br
    $subjectCount = $br.ReadInt32()
    $subjects = @()
    for ($s = 0; $s -lt $subjectCount; $s++) {
        $v = Read-MString $br
        $subjects += $v
        $totalChars += $v.Length
        $totalStrings++
        [void]$uniqueStrings.Add($v)
    }
    $contentCount = $br.ReadInt32()
    $contents = @()
    for ($c = 0; $c -lt $contentCount; $c++) {
        $v = Read-MString $br
        $contents += $v
        $totalChars += $v.Length
        $totalStrings++
        [void]$uniqueStrings.Add($v)
    }
    $totalChars += $ownerId.Length
    [void]$uniqueStrings.Add($ownerId)

    [void]$scripts.Add([PSCustomObject]@{
        ScriptID = $scriptId
        OwnerID = $ownerId
        Subjects = $subjects
        Contents = $contents
    })
}
} catch {
    Write-Output "FAILED at script index $i, stream position before this record: $posBefore, file length: $fileLen, error: $($_.Exception.Message)"
}

Write-Output "Total scripts: $totalScripts"
Write-Output "Total subject+content strings: $totalStrings"
Write-Output "Total unique strings (incl OwnerID): $($uniqueStrings.Count)"
Write-Output "Total characters (subject+content+ownerid): $totalChars"

$scripts | ConvertTo-Json -Depth 5 | Out-File -FilePath $OutPath -Encoding utf8
Write-Output "Wrote extract to $OutPath"
