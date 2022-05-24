import QtQuick 2.6
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.5

import ".."
import "../components"

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

Flickable {
    id: settingTab
    property alias cfg_Fps: sliderFps.value
    property alias cfg_Volume: sliderVol.value
    property alias cfg_MpvStats: ckbox_mpvStats.checked
    property alias cfg_VideoRate: spin_video_rate.dValue
    property alias cfg_MuteAudio: ckbox_muteAudio.checked
    property alias cfg_MouseInput: ckbox_mouseInput.checked
    property alias cfg_ResumeTime: resumeSpin.value
    property alias cfg_SwitchTimer: randomSpin.value
    property alias cfg_RandomizeWallpaper: ckbox_randomizeWallpaper.checked
    property alias cfg_PauseFilterByScreen: ckbox_pauseFilterByScreen.checked

    property int comboBoxWidth: themeWidth * 24

    Layout.fillWidth: true
    ScrollBar.vertical: ScrollBar { id: scrollbar }

    contentWidth: width - (scrollbar.visible ? scrollbar.width : 0)
    contentHeight: contentItem.childrenRect.height
    clip: true
    boundsBehavior: Flickable.OvershootBounds

    OptionGroup {
        header.visible: false
        anchors.left: parent.left
        anchors.right: parent.right


        OptionGroup {
            Layout.fillWidth: true
            header.text: 'Common Option'
            header.text_color: Theme.textColor
            header.icon: '../../images/cheveron-down.svg'
            header.color: Theme.activeBackgroundColor

            OptionItem {
                text: 'Pause'
                text_color: Theme.textColor
                icon: '../../images/pause.svg'
                actor:  ComboBox {
                    implicitWidth: comboBoxWidth
                    id: pauseMode
                    model: [
                        {
                            text: "Focus or Maximized Window",
                            value: Common.PauseMode.FocusOrMax
                        },
                        {
                            text: "Focus Window",
                            value: Common.PauseMode.Focus
                        },
                        {
                            text: "Maximized Window",
                            value: Common.PauseMode.Max
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
                contentBottom: ColumnLayout {
                    Text {
                        Layout.fillWidth: true
                        color: Theme.disabledTextColor
                        text: "Automatically pauses playback if any/focus/maximized window detected"
                        wrapMode: Text.Wrap
                    }
               }
            }
            OptionItem {
                text: 'Only check window on current screen'
                text_color: Theme.textColor
                actor: Switch {
                    id: ckbox_pauseFilterByScreen
                }
            }
            OptionItem {
                text: 'Display'
                text_color: Theme.textColor
                icon: '../../images/window.svg'
                actor: ComboBox {
                    id: displayMode
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
            }

            OptionItem {
                text: 'Resume Time'
                text_color: Theme.textColor
                icon: '../../images/timer.svg'
                actor: RowLayout {
                    spacing: 0
                    RowLayout {
                        SpinBox {
                            id: resumeSpin
                            from: 1
                            to: 60*1000
                            stepSize: 50
                        }
                        Label { text: " ms" }
                    }
                }
                contentBottom: ColumnLayout {
                    Text {
                        Layout.fillWidth: true
                        color: Theme.disabledTextColor
                        text: "Time to wait to resume playback from pause"
                    }
                }
            }
            OptionItem {
                text: 'Randomize Timer'
                text_color: Theme.textColor
                icon: '../../images/time.svg'
                actor: Switch {
                    id: ckbox_randomizeWallpaper
                }
                contentBottom: ColumnLayout {
                    Text {
                        Layout.fillWidth: true
                        color: Theme.disabledTextColor
                        text: "Randomize wallpapers filtered in the 'Wallpapers' page"
                    }
                    RowLayout {
                        visible: ckbox_randomizeWallpaper.checked
                        Label { 
                            id:heightpicker
                            text: "Randomize every " 
                        }
                        SpinBox {
                            id: randomSpin
                            width: font.pixelSize * 4
                            from: 1
                            to: 60*24*30
                            stepSize: 1
                        }
                        Label { text: " min" }
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            OptionItem {
                text: "Mute Audio"
                text_color: Theme.textColor
                icon: ckbox_muteAudio.checked
                    ? '../../images/volume-off.svg'
                    : '../../images/volume-up.svg'
                actor: Switch {
                    id: ckbox_muteAudio
                }
            }
            OptionItem {
                text: "Volume"
                text_color: Theme.textColor
                visible: !cfg_MuteAudio
                actor: RowLayout {
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
            }
 
            OptionItem {
                visible: libcheck.wallpaper
                text_color: Theme.textColor
                text: "Mouse Input"
                icon: '../../images/mouse.svg'
                actor: Switch {
                    id: ckbox_mouseInput
                }
            }
       }

        OptionGroup {
            Layout.fillWidth: true

            header.text: 'Video Option'
            header.text_color: Theme.textColor
            header.icon: '../../images/cheveron-down.svg'
            header.color: Theme.activeBackgroundColor

            OptionItem {
                text: 'Video Backend'
                text_color: Theme.textColor
                icon: '../../images/plugin.svg'
                actor: ComboBox {
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
            }
            
            OptionItem {
                text: "Video Playback Rate"
                text_color: Theme.textColor
                icon: '../../images/fast-forward.svg'
                actor: RowLayout {
                    DoubleSpinBox {
                        id: spin_video_rate
                        dFrom: 0.1
                        dTo: 100.0
                        dStepSize: 0.1
                    }
                }
            }

            OptionItem {
                text: 'Show Mpv Stats'
                text_color: Theme.textColor
                icon: '../../images/information-outline.svg'
                visible: cfg_VideoBackend == Common.VideoBackend.Mpv
                actor: Switch {
                    id: ckbox_mpvStats
                }
            }
        }
        OptionGroup {
            Layout.fillWidth: true

            header.text: 'Scene Option'
            header.text_color: Theme.textColor
            header.icon: '../../images/cheveron-down.svg'
            header.color: Theme.activeBackgroundColor
            visible: libcheck.wallpaper

            OptionItem {
                text: 'Fps'
                text_color: Theme.textColor
                icon: '../../images/tuning.svg'
                actor: RowLayout {
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
                    }
                }
                contentBottom: ColumnLayout {
                    Text {
                        Layout.fillWidth: true
                        color: Theme.disabledTextColor
                        text: "Low: 10, Medium: 15, High: 25, Ultra High: 30"
                    }
                }

            }
        }
    }


}

