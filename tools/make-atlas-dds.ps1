# Gera assets/textures/atlas.dds - a spritesheet unica do jogo (degrau 3).
#
# Mesma tecnica do make-invader-dds.ps1 do degrau 2 (DDS RGBA8 sem
# compressao, header DX10, lido direto pelo tinydds do ResourceLoader), agora
# com TODOS os sprites do jogo num unico atlas 64x32 em celulas de 16x16:
#
#   linha 0: crab f0 | crab f1 | squid f0 | squid f1
#   linha 1: octo f0 | octo f1 | player   | shot
#
# Cada sprite fica no canto superior esquerdo da sua celula; a folga
# transparente das celulas serve de guarda contra bleed do vizinho. Os dois
# frames por invasor ja preparam a animacao do degrau 4. Os pixels sao
# BRANCOS de proposito: a cor final vem do tint por vertice do batcher
# (drawSprite recebe a cor), como nos arcades com overlay de cor.
#
# As regioes em pixels correspondentes vivem em ForgeSpriteUi.h - mudou o
# atlas aqui, atualize la.
#
# Uso: powershell -File tools\make-atlas-dds.ps1  (a partir da raiz do repo)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$outDir = Join-Path $repoRoot 'assets\textures'
$outFile = Join-Path $outDir 'atlas.dds'
New-Item -ItemType Directory -Force $outDir | Out-Null

$crab0 = @(
    '..X.....X..',
    '...X...X...',
    '..XXXXXXX..',
    '.XX.XXX.XX.',
    'XXXXXXXXXXX',
    'X.XXXXXXX.X',
    'X.X.....X.X',
    '...XX.XX...'
)
$crab1 = @(
    '..X.....X..',
    'X..X...X..X',
    'X.XXXXXXX.X',
    'XXX.XXX.XXX',
    'XXXXXXXXXXX',
    '.XXXXXXXXX.',
    '..X.....X..',
    '.X.......X.'
)
$squid0 = @(
    '...XX...',
    '..XXXX..',
    '.XXXXXX.',
    'XX.XX.XX',
    'XXXXXXXX',
    '..X..X..',
    '.X.XX.X.',
    'X.X..X.X'
)
$squid1 = @(
    '...XX...',
    '..XXXX..',
    '.XXXXXX.',
    'XX.XX.XX',
    'XXXXXXXX',
    '.X.XX.X.',
    'X......X',
    '.X....X.'
)
$octo0 = @(
    '....XXXX....',
    '.XXXXXXXXXX.',
    'XXXXXXXXXXXX',
    'XXX..XX..XXX',
    'XXXXXXXXXXXX',
    '...XX..XX...',
    '..XX.XX.XX..',
    'XX........XX'
)
$octo1 = @(
    '....XXXX....',
    '.XXXXXXXXXX.',
    'XXXXXXXXXXXX',
    'XXX..XX..XXX',
    'XXXXXXXXXXXX',
    '..XXX..XXX..',
    '.XX..XX..XX.',
    '..XX....XX..'
)
$player = @(
    '......X......',
    '.....XXX.....',
    '.....XXX.....',
    '.XXXXXXXXXXX.',
    'XXXXXXXXXXXXX',
    'XXXXXXXXXXXXX',
    'XXXXXXXXXXXXX',
    'XXXXXXXXXXXXX'
)
$shot = @(
    '.X.',
    'X..',
    '.X.',
    '..X',
    '.X.',
    'X..',
    '.X.'
)

$texW = 64; $texH = 32
$cell = 16
$pixels = New-Object byte[] ($texW * $texH * 4)   # RGBA, ja zerado (transparente)

function Blit($sprite, $cellCol, $cellRow) {
    $ox = $cellCol * $script:cell
    $oy = $cellRow * $script:cell
    for ($row = 0; $row -lt $sprite.Count; $row++) {
        $line = $sprite[$row]
        for ($col = 0; $col -lt $line.Length; $col++) {
            if ($line[$col] -eq 'X') {
                $i = (($row + $oy) * $script:texW + ($col + $ox)) * 4
                $script:pixels[$i] = 255; $script:pixels[$i + 1] = 255
                $script:pixels[$i + 2] = 255; $script:pixels[$i + 3] = 255
            }
        }
    }
}

Blit $crab0  0 0
Blit $crab1  1 0
Blit $squid0 2 0
Blit $squid1 3 0
Blit $octo0  0 1
Blit $octo1  1 1
Blit $player 2 1
Blit $shot   3 1

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
