import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.5
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.4 as Kirigami

import ".."

Kirigami.FormLayout {
    id: settingTab
    property alias cfg_Fps: sliderFps.value
    property alias cfg_Volume: sliderVol.value
    property alias cfg_MpvStats: ckbox_mpvStats.checked
    property alias cfg_MuteAudio: ckbox_muteAudio.checked
    property alias cfg_MouseInput: ckbox_mouseInput.checked
    property alias cfg_SwitchTimer: randomSpin.value
    property alias cfg_RandomizeWallpaper: ckbox_randomizeWallpaper.checked

    property int comboBoxWidth: themeWidth * 24

    Layout.fillWidth: true
    twinFormLayouts: parentLayout
    ComboBox {
        Kirigami.FormData.label: "Pause:"
        implicitWidth: comboBoxWidth
        id: pauseMode
        model: [
            {
                text: "Maximized Window",
                value: Common.PauseMode.Max
            },
            {
                text: "Focus Window",
                value: Common.PauseMode.Focus
            },
            {
                text: "Any Window",
                value: Common.PauseMode.Any
            },
            {
                text: "Never",
                value: Common.PauseMode.Never
            }
        ]
        textRole: "text"
        onActivated: cfg_PauseMode = Common.cbCurrentValue(this)
        Component.onCompleted: currentIndex = Common.cbIndexOfValue(this, cfg_PauseMode)
    }
    ComboBox {
        id: displayMode
        Kirigami.FormData.label: "Display:"
        implicitWidth: comboBoxWidth
        model: [
            {
                text: "Keep Aspect Ratio",
                value: Common.DisplayMode.Aspect
            },
            {
                text: "Scale and Crop",
                value: Common.DisplayMode.Crop
            },
            {
                text: "Scale to Fill",
                value: Common.DisplayMode.Scale
            },
        ]
        textRole: "text"
        onActivated: cfg_DisplayMode = Common.cbCurrentValue(this)
        Component.onCompleted: currentIndex = Common.cbIndexOfValue(this, cfg_DisplayMode)
    }
    ComboBox {
        Kirigami.FormData.label: "Video backend:"
        implicitWidth: comboBoxWidth
        model: [
            {
                text: "QtMultimedia",
                value: Common.VideoBackend.QtMultimedia,
                enabled: true
            },
            {
                text: "Mpv",
                value: Common.VideoBackend.Mpv,
                enabled: libcheck.wallpaper
            }
        ].filter(el => el.enabled)
        textRole: "text"
        onActivated: cfg_VideoBackend = Common.cbCurrentValue(this)
        Component.onCompleted: currentIndex = Common.cbIndexOfValue(this, cfg_VideoBackend)
    }
    CheckBox {
        id: ckbox_muteAudio
        Kirigami.FormData.label: "Mute audio:"
    }
    RowLayout {
        spacing: 0
        Kirigami.FormData.label:  "Randomize:"
        CheckBox{
            id: ckbox_randomizeWallpaper
            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                subText: "randomize wallpapers showing in the Wallpaper page"
            }
        }
        RowLayout {
            enabled: cfg_RandomizeWallpaper
            Label { 
                id:heightpicker
                text: " every " 
            }
            SpinBox {
                id: randomSpin
                width: font.pixelSize * 4
                height: heightpicker.height
                from: 1
                to: 60*24*30
                stepSize: 1
            }
            Label { text: " min" }
        }
    }
    CheckBox {
        id: ckbox_mouseInput
        visible: libcheck.wallpaper
        Kirigami.FormData.label: "Mouse input:"
    }
    RowLayout {
        Kirigami.FormData.label:  "Volume:"
        visible: !cfg_MuteAudio
        Layout.preferredWidth: displayMode.width
        Label {
            Layout.preferredWidth: font.pixelSize * 2
            text: sliderVol.value.toString()
        }
        Slider {
            id: sliderVol
            Layout.fillWidth: true
            from: 0
            to: 100
            stepSize: 5.0
            snapMode: Slider.SnapOnRelease
        }
    }
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: "Mpv"
        Kirigami.FormData.labelAlignment: Qt.AlignLeft
    }
    CheckBox{
        Kirigami.FormData.label:  "Show stats"
        id: ckbox_mpvStats
    }
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: "Scene"
        Kirigami.FormData.labelAlignment: Qt.AlignTop
    }
    RowLayout {
        Kirigami.FormData.label:  "Fps:"
        Layout.preferredWidth: displayMode.width
        Label {
            Layout.preferredWidth: font.pixelSize * 2
            text: sliderFps.value.toString()
        }
        Slider {
            id: sliderFps
            Layout.fillWidth: true
            from: 5
            to: 60
            stepSize: 1.0
            snapMode: Slider.SnapOnRelease
            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                subText: "Control fps on scene wallpaper"
            }
        }
    }

}

