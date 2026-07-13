# Gera assets/textures/invader.dds - o sprite de teste do degrau 2.
#
# A distribuicao v1.63 do The-Forge nao traz conversor de imagem em Tools/
# (os .tex de Art/Textures/dds sao DDS renomeados), entao o tooling e nosso:
# um DDS RGBA8 *sem compressao* com header DX10, que o tinydds do
# ResourceLoader le direto (TinyImageFormat_R8G8B8A8_UNORM). Sem compressao
# de proposito: pixel art quer texel exato, nao blocos BC.
#
# Uso: powershell -File tools\make-invader-dds.ps1  (a partir da raiz do repo)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$outDir = Join-Path $repoRoot 'assets\textures'
$outFile = Join-Path $outDir 'invader.dds'
New-Item -ItemType Directory -Force $outDir | Out-Null

# Invasor classico (caranguejo) 11x8, centrado num canvas 16x16 com borda
# transparente - a borda tambem exercita o alpha.
$sprite = @(
    '..X.....X..',
    '...X...X...',
    '..XXXXXXX..',
    '.XX.XXX.XX.',
    'XXXXXXXXXXX',
    'X.XXXXXXX.X',
    'X.X.....X.X',
    '...XX.XX...'
)

$texW = 16; $texH = 16
$offX = 2; $offY = 4
$r = 64; $g = 255; $b = 96   # verde arcade

$pixels = New-Object byte[] ($texW * $texH * 4)   # RGBA, ja zerado (transparente)
for ($row = 0; $row -lt $sprite.Count; $row++) {
    $line = $sprite[$row]
    for ($col = 0; $col -lt $line.Length; $col++) {
        if ($line[$col] -eq 'X') {
            $i = (($row + $offY) * $texW + ($col + $offX)) * 4
            $pixels[$i] = $r; $pixels[$i + 1] = $g; $pixels[$i + 2] = $b; $pixels[$i + 3] = 255
        }
    }
}

$stream = [System.IO.File]::Create($outFile)
$w = New-Object System.IO.BinaryWriter($stream)
try {
    $w.Write([uint32]0x20534444)          # magic "DDS "
    # DDS_HEADER (124 bytes)
    $w.Write([uint32]124)                 # dwSize
    $w.Write([uint32]0x2100F)             # CAPS|HEIGHT|WIDTH|PITCH|PIXELFORMAT|MIPMAPCOUNT
    $w.Write([uint32]$texH)               # dwHeight
    $w.Write([uint32]$texW)               # dwWidth
    $w.Write([uint32]($texW * 4))         # dwPitchOrLinearSize (bytes por linha)
    $w.Write([uint32]0)                   # dwDepth
    $w.Write([uint32]1)                   # dwMipMapCount
    for ($i = 0; $i -lt 11; $i++) { $w.Write([uint32]0) }  # dwReserved1[11]
    # DDS_PIXELFORMAT (32 bytes): fourCC DX10 -> formato vem do header estendido
    $w.Write([uint32]32)                  # dwSize
    $w.Write([uint32]0x4)                 # DDPF_FOURCC
    $w.Write([uint32]0x30315844)          # 'DX10'
    for ($i = 0; $i -lt 5; $i++) { $w.Write([uint32]0) }   # masks (nao usadas)
    $w.Write([uint32]0x1000)              # dwCaps = DDSCAPS_TEXTURE
    for ($i = 0; $i -lt 4; $i++) { $w.Write([uint32]0) }   # caps2..4 + reserved2
    # DDS_HEADER_DXT10 (20 bytes)
    $w.Write([uint32]28)                  # DXGI_FORMAT_R8G8B8A8_UNORM
    $w.Write([uint32]3)                   # D3D10_RESOURCE_DIMENSION_TEXTURE2D
    $w.Write([uint32]0)                   # miscFlag
    $w.Write([uint32]1)                   # arraySize
    $w.Write([uint32]0)                   # miscFlags2 (alpha mode: unknown)
    $w.Write($pixels)
}
finally {
    $w.Dispose()
}

Write-Host "OK: $outFile ($((Get-Item $outFile).Length) bytes - esperado $(128 + 20 + $pixels.Length))"
