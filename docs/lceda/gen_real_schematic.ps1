# ============================================================
# demo3_ESP32 载板原理图生成器 v3
# 产出: 1) 立创EDA标准版工程JSON(可导入→原理图转PCB)
#       2) SVG 工程图 + PDF (Edge 无头打印)
# 设计: 载板式 — ESP32-S3-DevKitC-1 插在 J1 上, 板载 AMS1117 电源/
#       麦克风接口/功放接口/OLED 接口/舵机接口/光敏/触摸/按键
# ============================================================
$ErrorActionPreference = "Stop"
$script:gid = 0
function NewId { $script:gid++; return "gge$($script:gid)" }

# ---------- 网络表 ----------
$N3V3="+3V3"; $N5V="+5V"; $NGND="GND"; $N5VS="+5V_SERVO"

# ---------- 组件定义: name,title,x,y,w,h,side,pins[@(num,net显示名,net)] ----------
$script:comps = @()

function Add-Comp {
    param([string]$d,[string]$title,[int]$x,[int]$y,[int]$w,[int]$h,[string]$side,[object[]]$pins,[string]$sub=$title)
    $script:comps += @{d=$d; title=$title; sub=$sub; x=$x; y=$y; w=$w; h=$h; side=$side; pins=$pins}
}

# --- 电源 ---
Add-Comp "J12" "DC 5V IN" 60 140 120 60 "right" @(@("1","+5V",$N5V), @("2","GND",$NGND)) " screw terminal"
Add-Comp "U1" "AMS1117-3.3" 120 260 120 80 "lr" @(@("IN","+5V",$N5V), @("OUT","+3V3",$N3V3), @("GND","GND",$NGND)) "LDO SOT-223"
Add-Comp "C1" "10uF/25V" 120 420 90 40 "right" @(@("1","+5V",$N5V), @("2","GND",$NGND)) "input cap"
Add-Comp "C2" "22uF" 120 490 90 40 "right" @(@("1","+3V3",$N3V3), @("2","GND",$NGND)) "output cap"
Add-Comp "R4" "1K 0603" 260 420 90 40 "right" @(@("1","LED_A",$N3V3), @("2","LED_K","NLED")) "LED limit"
Add-Comp "LED1" "LED GREEN" 260 490 90 40 "right" @(@("A","LED_A","NLED"), @("K","LED_K",$NGND)) "power led"

# --- 按键 ---
Add-Comp "SW2" "RST(EN)" 60 620 100 50 "right" @(@("1","EN_RST","NEN"), @("2","GND",$NGND)) "tact"
Add-Comp "SW1" "BOOT(IO0)" 60 700 100 50 "right" @(@("1","BOOT0","NB0"), @("2","GND",$NGND)) "tact"
Add-Comp "R5" "10K 0603" 200 620 90 40 "right" @(@("1","EN_RST","NEN"), @("2","+3V3",$N3V3)) "EN pullup"
Add-Comp "R6" "10K 0603" 200 700 90 40 "right" @(@("1","BOOT0","NB0"), @("2","+3V3",$N3V3)) "IO0 pullup"

# --- 编程口 ---
Add-Comp "J14" "UART PROG" 60 800 120 140 "right" @(
    @("1","+3V3",$N3V3), @("2","UTX","UTX0"), @("3","URX","URX0"), @("4","EN_RST","NEN"), @("5","BOOT0","NB0"), @("6","GND",$NGND)
) "1x6 2.54 header"

# --- J1 DevKitC 载板主头(功能分组) ---
Add-Comp "J1" "ESP32-S3-DevKitC-1" 380 120 110 420 "right" @(
    @("1","3V3",$N3V3), @("2","GND",$NGND),
    @("3","EN_RST","NEN"), @("4","BOOT0","NB0"),
    @("5","GPIO4","MIC_SCK"), @("6","GPIO5","MIC_WS"), @("7","GPIO6","MIC_SD"),
    @("8","GPIO7","SPK_DIN"), @("9","GPIO15","SPK_BCLK"), @("10","GPIO16","SPK_LRC"),
    @("11","GPIO8","SDA"), @("12","GPIO9","SCL"),
    @("13","GPIO11","SERVO_PAN"), @("14","GPIO12","SERVO_TILT"),
    @("15","GPIO2","LDR_ADC"), @("16","GPIO14","TOUCH"),
    @("17","GPIO47","NLED"), @("18","UTX","UTX0"), @("19","URX","URX0")
) "插板主头(功能序)"

# --- 音频 ---
Add-Comp "J3" "INMP441 MIC" 620 120 140 140 "left" @(
    @("VDD","3V3"), @("GND","GND"), @("L/R","GND"),
    @("SCK","MIC_SCK"), @("WS","MIC_WS"), @("SD","MIC_SD")
) "1x6 2.54"
Add-Comp "C3" "100nF" 820 120 90 40 "right" @(@("1","3V3",$N3V3), @("2","GND",$NGND)) "mic decoupling"
Add-Comp "J4" "MAX98357A AMP" 620 320 150 200 "left" @(
    @("VIN","5V"), @("GND","GND"), @("SD","3V3"), @("GAIN","NC_G"),
    @("DIN","SPK_DIN"), @("BCLK","SPK_BCLK"), @("LRC","SPK_LRC"),
    @("OUT+","SPK+"), @("OUT-","SPK-")
) "1x9 2.54"
Add-Comp "J5" "SPEAKER 8R3W" 620 580 150 60 "left" @(@("1","SPK+","NC_SPK"), @("2","SPK-","NC_SPK")) "2P connector"

# --- 显示 ---
Add-Comp "J6" "OLED SSD1306" 620 680 140 100 "left" @(
    @("VCC","3V3"), @("GND","GND"), @("SCL","SCL"), @("SDA","SDA")
) "1x4 2.54"
Add-Comp "R2" "4.7K 0603" 820 680 90 40 "right" @(@("1","SCL",$N3V3), @("2","SCL","NC_SCL")) "I2C pullup"
Add-Comp "R3" "4.7K 0603" 820 750 90 40 "right" @(@("1","SDA",$N3V3), @("2","SDA","NC_SDA")) "I2C pullup"

# --- 传感器 ---
Add-Comp "J7" "LDR GL5528" 620 820 140 60 "left" @(@("TOP","3V3",$N3V3), @("BOT","LDR_ADC","NLDR")) ""
Add-Comp "R1" "10K 0603" 620 910 90 40 "right" @(@("1","LDR_ADC","NLDR"), @("2","GND",$NGND)) ""
Add-Comp "J8" "TOUCH FOIL" 820 820 130 50 "left" @(@("T","TOUCH","NTOUCH")) "1P"

# --- 舵机 ---
Add-Comp "J15" "SERVO 5V IN" 620 990 150 60 "left" @(@("1","+5V_SERVO",$N5VS), @("2","GND",$NGND)) "screw terminal"
Add-Comp "J9" "SG90 pan" 900 990 130 60 "left" @(@("SIG","SERVO_PAN","NSP"), @("VCC","+5V_SERVO",$N5VS), @("GND","GND",$NGND)) ""
Add-Comp "J10" "SG90 tilt" 900 1090 130 60 "left" @(@("SIG","SERVO_TILT","NST"), @("VCC","+5V_SERVO",$N5VS), @("GND","GND",$NGND)) ""

# ============ 产 A: 立创EDA JSON ============
$script:gid = 0
$shapes = New-Object System.Collections.Generic.List[string]
function Add-Pin([int]$dotX,[int]$dotY,[string]$num,[string]$name,[string]$side){
    $id = NewId
    if ($side -eq "right") {
        $shapes.Add("P~show~0~$num~$dotX~$dotY~0~$id^^$dotX~$dotY^^M $dotX $dotY h 20~#880000^^1~$($dotX-4)~$($dotY+4)~0~$name~end~~9pt^^1~$($dotX+8)~$($dotY-5)~0~$num~start~~9pt^^0~$($dotX+10)~$dotY^^0~M $($dotX+17) $($dotY-3) L $($dotX+20) $dotY L $($dotX+17) $($dotY+3)")
    } else {
        $shapes.Add("P~show~0~$num~$dotX~$dotY~0~$id^^$dotX~$dotY^^M $dotX $dotY h -20~#880000^^1~$($dotX+4)~$($dotY+4)~0~$name~start~~9pt^^1~$($dotX-8)~$($dotY-5)~0~$num~end~~9pt^^0~$($dotX-10)~$dotY^^0~M $($dotX-17) $($dotY-3) L $($dotX-20) $dotY L $($dotX-17) $($dotY+3)")
    }
}
function Add-NetLabel([int]$x,[int]$y,[string]$net,[string]$anchor){
    $id = NewId
    $color = if ($net -in @("+3V3","+5V","+5V_SERVO")) {"#FF0000"} elseif ($net -eq "GND") {"#000000"} else {"#1D2064"}
    $tx = if ($anchor -eq "start") { $x+2 } else { $x-2 }
    $shapes.Add("N~$x~$y~0~$color~$net~$id~$anchor~$tx~$y~Times New Roman~")
}
function Add-T([int]$x,[int]$y,[string]$text,[string]$color="#000000",[string]$size="9pt"){
    $shapes.Add("T~L~$x~$y~0~$color~~$size~bold~normal~~comment~$text~1~start~$(NewId)")
}
function Add-Rect([int]$x,[int]$y,[int]$w,[int]$h){
    $shapes.Add("R~$x~$y~~~$w~$h~#000000~1~0~none~$(NewId)")
}
function Add-Wire([string]$pts){ $shapes.Add("W~$pts~#000000~1~0~none~$(NewId)~0") }

foreach ($c in $script:comps) {
    Add-Rect $c.x $c.y $c.w $c.h
    Add-T ($c.x+5) ($c.y+14) "$($c.d)  $($c.title)"
    if ($c.sub -ne $c.title) { Add-T ($c.x+5) ($c.y+$c.h-4) $c.sub 7 "#64748b" }
    $i = 0
    foreach ($p in $c.pins) {
        $i++
        $py = $c.y + 20 + 20*($i-1)
        if ($py -ge ($c.y + $c.h)) { $py = $c.y + $c.h - 10 }
        if ($c.side -eq "right") {
            $dotX = $c.x + $c.w
            Add-Pin $dotX $py $i $p[0] "right"
            Add-NetLabel ($dotX+20) $py $p[2] "start"
        } elseif ($c.side -eq "left") {
            $dotX = $c.x
            Add-Pin $dotX $py $i $p[0] "left"
            Add-NetLabel ($dotX-20) $py $p[2] "end"
        } else { # lr: IN 左出 OUT 右出 GND 底
            if ($i -eq 1) { Add-Pin $c.x $py $i $p[0] "left";  Add-NetLabel ($c.x-20) $py $p[2] "end" }
            elseif ($i -eq 2) { $rx = $c.x+$c.w; Add-Pin $rx $py $i $p[0] "right"; Add-NetLabel ($rx+20) $py $p[2] "start" }
            else { $bx = $c.x + [int]($c.w/2); Add-Pin $bx ($c.y+$c.h) $i $p[0] "bottom"; Add-NetLabel ($bx+20) ($c.y+$c.h) $p[2] "start" }
        }
    }
}
# 分区标题
Add-T 60 100 "① POWER  5V in → AMS1117-3.3 → 3V3 rail" 10 "#B91C1C"
Add-T 380 100 "② MCU  ESP32-S3-DevKitC-1 (plug onto J1)" 10 "#1D4ED8"
Add-T 620 100 "③ AUDIO  MIC + AMP" 10 "#6D28D9"
Add-T 620 660 "④ DISPLAY" 10 "#0F766E"
Add-T 620 800 "⑤ SENSOR" 10 "#15803D"
Add-T 620 970 "⑥ SERVO  external 5V only!" 10 "#B45309"
# 3V3/5V 汇流线(视觉)
Add-Wire "60 60 1060 60"
Add-T 1064 64 "+3V3 rail" 9 "#FF0000"
Add-Wire "60 80 1060 80"
Add-T 1064 84 "GND rail" 9 "#000000"

$doc = [ordered]@{
    editorVersion = "6.5.42"; docType = 5
    title = "demo3_ESP32 robot carrier"
    description = "ESP32-S3 DevKitC carrier: AMS1117 power, INMP441 mic, MAX98357A amp, OLED, servos, LDR, touch"
    colors = ""
    schematics = @([ordered]@{
        docType = 1; title = "Sheet_1"; description = ""
        dataStr = [ordered]@{
            head = [ordered]@{
                docType="1"; editorVersion="6.5.42"; newgId=$true
                c_para=[ordered]@{"Prefix Start"="1"}; c_spiceCmd="null"; hasIdFlag=$true
                uuid="d3m03carrier5heet1b1"; x="0"; y="0"
                portOfADImportHack=""; importFlag=0; transformList=""
            }
            canvas = "CA~1400~1300~#FFFFFF~yes~#CCCCCC~10~1400~1300~line~10~pixel~5~0~0"
            shape = $shapes
            BBox = [ordered]@{x=0;y=0;width=1400;height=1300}
            colors = ""
        }
    })
}
$outJson = "D:\project-base\demo3_ESP32\docs\lceda\demo3_robot_s3_schematic.json"
$doc | ConvertTo-Json -Depth 8 | Set-Content $outJson -Encoding UTF8
$chk = Get-Content $outJson -Raw -Encoding UTF8 | ConvertFrom-Json
"JSON OK: shapes=$($chk.schematics[0].dataStr.shape.Count)"

# ============ 产 B: SVG (同布局渲染) ============
$svg = New-Object System.Collections.Generic.List[string]
$svg.Add('<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1400 1300" font-family="Consolas,monospace" width="1400" height="1300">')
$svg.Add('<rect width="1400" height="1300" fill="white"/>')
foreach ($c in $script:comps) {
    $svg.Add("<rect x='$($c.x)' y='$($c.y)' width='$($c.w)' height='$($c.h)' fill='white' stroke='black' stroke-width='1.5'/>")
    $svg.Add("<text x='$($c.x+5)' y='$($c.y+14)' font-size='10' font-weight='bold' fill='#111'>$($c.d)  $($c.title)</text>")
    if ($c.sub -ne $c.title) { $svg.Add("<text x='$($c.x+5)' y='$($c.y+$c.h-4)' font-size='8' fill='#64748b'>$($c.sub)</text>") }
    $i = 0
    foreach ($p in $c.pins) {
        $i++
        $py = $c.y + 20 + 20*($i-1); if ($py -ge ($c.y+$c.h)) { $py = $c.y+$c.h-10 }
        if ($c.side -eq "right") {
            $dx = $c.x + $c.w
            $svg.Add("<line x1='$dx' y1='$py' x2='$($dx+20)' y2='$py' stroke='black' stroke-width='1.5'/><circle cx='$dx' cy='$py' r='2' fill='black'/>")
            $svg.Add("<text x='$($dx-4)' y='$($py+4)' font-size='9' text-anchor='end' fill='#111'>$($p[0])</text>")
            $clr = if ($p[2] -in @("+3V3","+5V","+5V_SERVO")) {"#dc2626"} elseif ($p[2] -eq "GND") {"#000"} else {"#1d2064"}
            $svg.Add("<text x='$($dx+22)' y='$($py+4)' font-size='9' font-weight='bold' fill='$clr'>$($p[2])</text>")
        } elseif ($c.side -eq "left") {
            $dx = $c.x
            $svg.Add("<line x1='$($dx-20)' y1='$py' x2='$dx' y2='$py' stroke='black' stroke-width='1.5'/><circle cx='$dx' cy='$py' r='2' fill='black'/>")
            $svg.Add("<text x='$($dx+4)' y='$($py+4)' font-size='9' fill='#111'>$($p[0])</text>")
            $clr = if ($p[2] -in @("+3V3","+5V","+5V_SERVO")) {"#dc2626"} elseif ($p[2] -eq "GND") {"#000"} else {"#1d2064"}
            $svg.Add("<text x='$($dx-22)' y='$($py+4)' font-size='9' font-weight='bold' text-anchor='end' fill='$clr'>$($p[2])</text>")
        } else {
            if ($i -eq 1) {
                $svg.Add("<line x1='$($c.x-20)' y1='$py' x2='$($c.x)' y2='$py' stroke='black' stroke-width='1.5'/><circle cx='$($c.x)' cy='$py' r='2' fill='black'/>")
                $svg.Add("<text x='$($c.x-22)' y='$($py+4)' font-size='9' font-weight='bold' text-anchor='end' fill='#dc2626'>$($p[2])</text>")
            } elseif ($i -eq 2) {
                $rx = $c.x+$c.w
                $svg.Add("<line x1='$rx' y1='$py' x2='$($rx+20)' y2='$py' stroke='black' stroke-width='1.5'/><circle cx='$rx' cy='$py' r='2' fill='black'/>")
                $svg.Add("<text x='$($rx+22)' y='$($py+4)' font-size='9' font-weight='bold' fill='#dc2626'>$($p[2])</text>")
            } else {
                $bx = $c.x + [int]($c.w/2)
                $svg.Add("<line x1='$bx' y1='$($c.y+$c.h)' x2='$bx' y2='$($c.y+$c.h+20)' stroke='black' stroke-width='1.5'/><circle cx='$bx' cy='$($c.y+$c.h)' r='2' fill='black'/>")
                $svg.Add("<text x='$($bx+22)' y='$($c.y+$c.h+24)' font-size='9' font-weight='bold' fill='#000'>$($p[2])</text>")
            }
        }
    }
}
$svg.Add("<line x1='60' y1='60' x2='1060' y2='60' stroke='#dc2626' stroke-width='2'/><text x='1064' y='64' font-size='10' fill='#dc2626'>+3V3 rail</text>")
$svg.Add("<line x1='60' y1='80' x2='1060' y2='80' stroke='black' stroke-width='2'/><text x='1064' y='84' font-size='10'>GND rail</text>")
foreach ($tt in @(@("① POWER  5V→AMS1117-3.3→3V3",60,100,"#B91C1C"), @("② MCU ESP32-S3-DevKitC-1 (plug onto J1)",380,100,"#1D4ED8"), @("③ AUDIO MIC+AMP",620,100,"#6D28D9"), @("④ DISPLAY",620,660,"#0F766E"), @("⑤ SENSOR",620,800,"#15803D"), @("⑥ SERVO external 5V only!",620,970,"#B45309"))) {
    $svg.Add("<text x='$($tt[1])' y='$($tt[2])' font-size='12' font-weight='bold' fill='$($tt[3])'>$($tt[0])</text>")
}
$svg.Add("<text x='60' y='1265' font-size='12' font-weight='bold'>demo3_ESP32 · ESP32-S3 Voice Robot Carrier Board · Sheet 1/1</text>")
$svg.Add("<text x='760' y='1265' font-size='11' fill='#64748b'>Rev 0.1 · 2026-11-06 · nets by label · servos powered from external 5V (J15), common GND</text>")
$svg.Add("</svg>")
$outSvg = "D:\project-base\demo3_ESP32\docs\schematic_pdf\demo3_robot_s3_schematic.svg"
New-Item -ItemType Directory -Force (Split-Path $outSvg) | Out-Null
$svg | Set-Content $outSvg -Encoding UTF8
"SVG OK -> $outSvg"

# ============ 产 C: PDF (Edge 无头打印) ============
$html = @"
<!DOCTYPE html><html><head><meta charset='utf-8'><style>
@page { size: 1400px 1300px; margin: 0; }
body { margin: 0; }
svg { display:block; }
</style></head><body>
$(($svg -join "`n"))
</body></html>
"@
$outHtml = "D:\project-base\demo3_ESP32\docs\schematic_pdf\demo3_robot_s3_schematic.html"
[System.IO.File]::WriteAllText($outHtml, $html, (New-Object Text.UTF8Encoding $true))
$edge = @("C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe", "C:\Program Files\Microsoft\Edge\Application\msedge.exe", "C:\Program Files\Google\Chrome\Application\chrome.exe") | Where-Object { Test-Path $_ } | Select-Object -First 1
if ($edge) {
    $outPdf = "D:\project-base\demo3_ESP32\docs\schematic_pdf\demo3_robot_s3_schematic.pdf"
    & $edge --headless --disable-gpu --no-pdf-header-footer --print-to-pdf="$outPdf" "file:///$($outHtml -replace '\\','/')" 2>$null | Out-Null
    Start-Sleep 3
    if (Test-Path $outPdf) { "PDF OK: {0:N0} KB" -f ((Get-Item $outPdf).Length/1KB) } else { "PDF 未生成 (Edge 超时), HTML 已就绪可手动打印" }
} else { "未找到 Edge/Chrome, HTML 已就绪: $outHtml" }
