# 生成立创EDA标准版 JSON 原理图（可导入专业版 → 原理图转PCB）
# 用法: powershell -File gen_lceda_json.ps1
$ErrorActionPreference = "Stop"
$shapes = New-Object System.Collections.Generic.List[object]

function Pin($x,$y,$num,$name,$part) {
    $shapes.Add([ordered]@{
        type="PIN"; x=$x; y=$y
        attributes=[ordered]@{".api_"=1; ".do"=1}
        pinNumber=$num; pinName=$name; pinDotVisible=1; pinClock=0
        pinSpotSize=2; pinSquare=1; pinExtend=20; pinHide=0
        pinNet=""; pinPart=$part
    })
}
function Rect($x,$y,$w,$h,$part) {
    $shapes.Add([ordered]@{
        type="RECT"; x=$x; y=$y; width=$w; height=$h
        fill="none"; stroke="#000000"; strokeWidth=1
        attributes=[ordered]@{".part"=$part; ".name"=$part}
    })
}
function Text($x,$y,$t,$size=9,$fill="#000000") {
    $shapes.Add([ordered]@{
        type="TEXT"; x=$x; y=$y; text=$t; fill=$fill; fontSize=$size
        rotation=0; attributes=[ordered]@{".name"=1}
    })
}
function Wire($x1,$y1,$x2,$y2,$net) {
    $shapes.Add([ordered]@{
        type="WIRE"; x1=$x1; y1=$y1; x2=$x2; y2=$y2
        stroke="#000000"; strokeWidth=1; net=$net
    })
}
function NetFlag($x,$y,$net,$rot=0) {
    $shapes.Add([ordered]@{
        type="NETFLAG"; x=$x; y=$y
        attributes=[ordered]@{".net"=1; ".netflag"=1}
        net=$net; text=$net
        color= if ($net -in @("3V3","5V")) {"#FF0000"} elseif ($net -eq "GND") {"#000000"} else {"#1D2064"}
        fontSize=7; rotation=$rot
    })
}
# 连接器: rect + 引脚(右出线 20px + 网络标签) 或 (左出线)
function Connector($name,$title,$rx,$ry,$rw,$side,$pins) {
    Rect $rx $ry $rw ((($pins.Count)*20)+20) $name
    Text ($rx+8) ($ry+16) $title 9
    $i = 0
    foreach ($p in $pins) {
        $i++
        $py = $ry + 20 + 20*($i-1)
        if ($side -eq "right") {
            $px = $rx + $rw
            Pin $px $py $i $p[0] $name
            Wire $px $py ($px+20) $py $p[1]
            NetFlag ($px+20) ($py+6) $p[1]
        } else {
            $px = $rx
            Pin $px $py $i $p[0] $name
            Wire $px $py ($px-20) $py $p[1]
            NetFlag ($px-20) ($py+6) $p[1] 180
        }
    }
}

# ===== J1 ESP32-S3-DevKitC（所用引脚引出）=====
Connector "J1" "ESP32-S3-DevKitC-1 (用到的引脚)" 300 120 90 "right" @(
    @("3V3","3V3"), @("5V","5V"), @("GND","GND"),
    @("GPIO8/SDA","SDA"), @("GPIO9/SCL","SCL"),
    @("GPIO4/MIC_SCK","MIC_SCK"), @("GPIO5/MIC_WS","MIC_WS"), @("GPIO6/MIC_SD","MIC_SD"),
    @("GPIO7/SPK_DIN","SPK_DIN"), @("GPIO15/SPK_BCLK","SPK_BCLK"), @("GPIO16/SPK_LRC","SPK_LRC"),
    @("GPIO11/SERVO_PAN","SERVO_PAN"), @("GPIO12/SERVO_TILT","SERVO_TILT"),
    @("GPIO2/LDR_ADC","LDR_ADC"), @("GPIO14/TOUCH","TOUCH"), @("GPIO47/LED","LED")
)

# ===== J2 INMP441 =====
Connector "J2" "INMP441 麦克风" 700 120 160 "left" @(
    @("VDD","3V3"), @("GND","GND"), @("L/R","GND"),
    @("SCK","MIC_SCK"), @("WS","MIC_WS"), @("SD","MIC_SD")
)

# ===== J3 MAX98357A =====
Connector "J3" "MAX98357A 功放" 700 360 150 "left" @(
    @("VIN","5V"), @("GND","GND"), @("SD","3V3"), @("GAIN","NC"),
    @("DIN","SPK_DIN"), @("BCLK","SPK_BCLK"), @("LRC","SPK_LRC"),
    @("OUT+","SPK+"), @("OUT-","SPK-")
)

# ===== J4 OLED =====
Connector "J4" "OLED SSD1306" 700 600 140 "left" @(
    @("VCC","3V3"), @("GND","GND"), @("SCL","SCL"), @("SDA","SDA")
)

# ===== J5/J6 舵机 =====
Connector "J5" "SG90 水平pan" 700 760 140 "left" @(
    @("SIG","SERVO_PAN"), @("VCC","5V"), @("GND","GND")
)
Connector "J6" "SG90 垂直tilt" 700 900 140 "left" @(
    @("SIG","SERVO_TILT"), @("VCC","5V"), @("GND","GND")
)

# ===== J7 光敏 + R1 =====
Connector "J7" "LDR 光敏" 120 560 130 "right" @(
    @("TOP","3V3"), @("BOT","LDR_ADC")
)
Connector "R1" "R1 10K 0603" 120 660 130 "right" @(
    @("1","LDR_ADC"), @("2","GND")
)

# ===== J8 触摸 =====
Connector "J8" "触摸线(T14)" 120 780 130 "right" @(
    @("TOUCH","TOUCH")
)

# ===== 说明文字 =====
Text 700 60 "demo3_ESP32 · ESP32-S3 语音版 · 模块级原理图 (立创EDA标准版JSON)" 12
Text 700 84 "导入: 立创EDA专业版 → 文件 → 导入 → 立创EDA标准版。J1 竖排排针代表 DevKitC 所用引脚" 8 "#64748b"
Text 700 104 "供电: 板子 USB-C 自供电; 舵机 VCC 必须外接 5V>=2A 且与板 GND 共地 (J5/J6)" 8 "#dc2626"

$doc = [ordered]@{
    HEAD = [ordered]@{ version = "2019.1002"; editorVersion = "v6.5" }
    CANVAS = [ordered]@{ width = "1100"; height = "1000" }
    shape = $shapes
}
$out = "D:\project-base\demo3_ESP32\docs\lceda\demo3_robot_s3_schematic.json"
New-Item -ItemType Directory -Force (Split-Path $out) | Out-Null
$doc | ConvertTo-Json -Depth 6 | Set-Content -Path $out -Encoding UTF8
# 验证
$check = Get-Content $out -Raw | ConvertFrom-Json
"OK: $($check.shape.Count) shapes -> $out"
