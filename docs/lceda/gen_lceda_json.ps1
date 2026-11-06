# 生成立创EDA标准版(编辑器格式)JSON原理图 — 依据官方文档 docs.easyeda.com/DocumentFormat/2-EasyEDA-Schematic-File-Format
# 导入: 立创EDA专业版 → 文件 → 导入 → 立创EDA标准版
$ErrorActionPreference = "Stop"
$script:gid = 0
function NewId { $script:gid++; return "gge$($script:gid)" }

$shapes = New-Object System.Collections.Generic.List[string]

# 引脚: P~show~electric~spiceNum~dotX~dotY~rotation~id^^dotX~dotY^^path~color^^name段^^number段^^dot段^^clock段
# 右侧引脚: dot在框右缘, path 朝外 20px; 左侧: path 朝左
function Add-Pin([int]$dotX, [int]$dotY, [string]$num, [string]$name, [string]$part, [string]$side) {
    $id = NewId
    if ($side -eq "right") {
        $path = "M $dotX $dotY h 20"
        $nameX = $dotX - 4; $numX = $dotX + 8
        $nameAnchor = "end"; $numAnchor = "start"
        $dotCkX = $dotX + 10
    } else {
        $path = "M $dotX $dotY h -20"
        $nameX = $dotX + 4; $numX = $dotX - 8
        $nameAnchor = "start"; $numAnchor = "end"
        $dotCkX = $dotX - 10
    }
    $s = "P~show~0~$num~$dotX~$dotY~0~$id" +
         "^^$dotX~$dotY" +
         "^^$path~#880000" +
         "^^1~$nameX~$($dotY+4)~0~$name~$nameAnchor~~9pt" +
         "^^1~$numX~$($dotY-5)~0~$num~$numAnchor~~9pt" +
         "^^0~$dotCkX~$dotY" +
         "^^0~M $($dotX+17) $($dotY-3) L $($dotX+20) $dotY L $($dotX+17) $($dotY+3)"
    $shapes.Add($s)
}

# 网络标签: N~dotX~dotY~rotation~color~name~id~anchor~textX~textY~font~
function Add-NetLabel([int]$x, [int]$y, [string]$net, [string]$anchor) {
    $id = NewId
    $color = if ($net -in @("3V3","5V")) { "#FF0000" } elseif ($net -eq "GND") { "#000000" } else { "#1D2064" }
    $tx = if ($anchor -eq "start") { $x + 2 } else { $x - 2 }
    $shapes.Add("N~$x~$y~0~$color~$net~$id~$anchor~$tx~$y~Times New Roman~")
}

# 文字: T~L~x~y~rot~color~font~size~weight~style~baseline~type~text~visible~anchor~id
function Add-Text([int]$x, [int]$y, [string]$text, [string]$color = "#000000", [string]$size = "9pt") {
    $id = NewId
    $shapes.Add("T~L~$x~$y~0~$color~~$size~bold~normal~~comment~$text~1~start~$id")
}

# 连接器: 外框 + 每引脚(P引脚 + 短导线W + 网络标签N)
function Add-Connector {
    param(
        [string]$Name, [string]$Title,
        [int]$X, [int]$Y, [int]$W, [string]$Side,
        [object[]]$Pins   # 每项 @(pinName, net)
    )
    $h = ($Pins.Count) * 20 + 20
    $id = NewId
    $shapes.Add("R~$X~$Y~~~$W~$h~#000000~1~0~none~$id")
    Add-Text ($X+6) ($Y+14) "$Name  $Title"
    $i = 0
    foreach ($p in $Pins) {
        $i++
        $py = $Y + 20 + 20*($i-1)
        if ($Side -eq "right") {
            $dotX = $X + $W
            Add-Pin $dotX $py $i $p[0] $Name "right"
            $ex = $dotX + 20
            Add-NetLabel $ex $py $p[1] "start"
        } else {
            $dotX = $X
            Add-Pin $dotX $py $i $p[0] $Name "left"
            $ex = $dotX - 20
            Add-NetLabel $ex $py $p[1] "end"
        }
    }
}

# ================= J1 ESP32-S3-DevKitC-1 (使用的引脚) =================
Add-Connector "J1" "ESP32-S3-DevKitC-1" 300 120 100 "right" @(
    @("3V3","3V3"), @("5V","5V"), @("GND","GND"),
    @("GPIO8/SDA","SDA"), @("GPIO9/SCL","SCL"),
    @("GPIO4/MIC_SCK","MIC_SCK"), @("GPIO5/MIC_WS","MIC_WS"), @("GPIO6/MIC_SD","MIC_SD"),
    @("GPIO7/SPK_DIN","SPK_DIN"), @("GPIO15/SPK_BCLK","SPK_BCLK"), @("GPIO16/SPK_LRC","SPK_LRC"),
    @("GPIO11/SERVO_PAN","SERVO_PAN"), @("GPIO12/SERVO_TILT","SERVO_TILT"),
    @("GPIO2/LDR_ADC","LDR_ADC"), @("GPIO14/TOUCH","TOUCH"), @("GPIO47/LED","LED")
)

# ================= J2 INMP441 =================
Add-Connector "J2" "INMP441 mic" 720 120 150 "left" @(
    @("VDD","3V3"), @("GND","GND"), @("L/R","GND"),
    @("SCK","MIC_SCK"), @("WS","MIC_WS"), @("SD","MIC_SD")
)

# ================= J3 MAX98357A =================
Add-Connector "J3" "MAX98357A amp" 720 340 150 "left" @(
    @("VIN","5V"), @("GND","GND"), @("SD","3V3"), @("GAIN","NC"),
    @("DIN","SPK_DIN"), @("BCLK","SPK_BCLK"), @("LRC","SPK_LRC"),
    @("OUT+","SPK+"), @("OUT-","SPK-")
)

# ================= J4 OLED =================
Add-Connector "J4" "OLED SSD1306" 720 600 140 "left" @(
    @("VCC","3V3"), @("GND","GND"), @("SCL","SCL"), @("SDA","SDA")
)

# ================= J5/J6 舵机 =================
Add-Connector "J5" "SG90 pan" 720 720 130 "left" @(
    @("SIG","SERVO_PAN"), @("VCC","5V"), @("GND","GND")
)
Add-Connector "J6" "SG90 tilt" 720 860 130 "left" @(
    @("SIG","SERVO_TILT"), @("VCC","5V"), @("GND","GND")
)

# ================= J7 LDR + R1 =================
Add-Connector "J7" "LDR" 120 560 130 "right" @(
    @("TOP","3V3"), @("BOT","LDR_ADC")
)
Add-Connector "R1" "R1 10K 0603" 120 660 130 "right" @(
    @("1","LDR_ADC"), @("2","GND")
)

# ================= J8 触摸 =================
Add-Connector "J8" "Touch foil (T14/GPIO14)" 120 780 130 "right" @(
    @("TOUCH","TOUCH")
)

# 说明
Add-Text 300 90 "demo3_ESP32 v0.5 · ESP32-S3 voice robot · module-level schematic (EasyEDA std JSON)" 12 "#1D2064"
Add-Text 300 106 "Board power: USB-C. Servo VCC (J5/J6) MUST be external 5V>=2A, common GND with board." 8 "#dc2626"
Add-Text 720 90 "modules: J2 mic / J3 amp / J4 oled / J5 J6 servos" 8 "#64748b"

$doc = [ordered]@{
    head   = "1~1.7.5~~"
    canvas = "CA~1200~1200~`#FFFFFF~yes~`#CCCCCC~10~1200~1200~line~10~pixel~5~400~300"
    shape  = $shapes
}
$out = "D:\project-base\demo3_ESP32\docs\lceda\demo3_robot_s3_schematic.json"
$doc | ConvertTo-Json -Depth 4 | Set-Content -Path $out -Encoding UTF8
$check = Get-Content $out -Raw -Encoding UTF8 | ConvertFrom-Json
"OK: head=[$($check.head)] canvas=[$($check.canvas)] shapes=$($check.shape.Count) -> $out"
$check.shape | Select-Object -First 3
