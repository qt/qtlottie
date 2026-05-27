// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.lottieqt

ColumnLayout {
    id: top
    spacing: 10
    property string source: ""

    Rectangle {
        color: "lightgray"
        border.color: "black"
        border.width: 2
        radius: 10
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        LottieAnimation {
            id: qtlottie
            transformOrigin: Item.TopLeft
            x: (textureSize.width > parent.width) ? 0 : ((parent.width - textureSize.width) / 2)
            y: (textureSize.height > parent.height) ? 0 : ((parent.height - textureSize.height) / 2)

            source: top.source
            quality: LottieAnimation.HighQuality
            loops: LottieAnimation.Infinite
            textureSize: Qt.size(width * scale, height * scale)
            fillColor: "#F0F0F0"
            onStatusChanged: {
                if (status == LottieAnimation.Ready) {
                    let newScale = Math.min(parent.width / (width * 1.2), parent.height / (height * 1.2))
                    scaleSlider.from = newScale * 10
                    scaleSlider.to = newScale * 200
                    scaleSlider.value = newScale * 100
                    playButton.checked = false
                }
            }
        }
    }

    RowLayout {
        id: controlRow
        spacing: top.spacing

        Button {
            id: openFileButton
            text: "Open..."
            onClicked: {
                fileDialog.visible = true
            }
            Layout.preferredHeight: aGroupBox.height
        }

        Button {
            id: playButton
            enabled: (qtlottie.status === LottieAnimation.Ready)
            checkable: true
            text: checked ? "Play" : "Pause"
            onClicked: qtlottie.togglePause()
            Layout.preferredHeight: aGroupBox.height
        }

        GroupBox {
            id: frameBox
            title: "Current Frame: " + qtlottie.currentFrame
            enabled: playButton.checked && (qtlottie.status === LottieAnimation.Ready)
            contentItem: RowLayout {
                Text { text: qtlottie.startFrame }
                Slider {
                    id: gotoSlider
                    from: qtlottie.startFrame
                    to: qtlottie.endFrame
                    stepSize: 1
                    value: 0
                    onValueChanged: { if (frameBox.enabled) qtlottie.gotoAndStop(value) }
                    Binding on value {
                        when: !frameBox.enabled
                        value: qtlottie.currentFrame
                        restoreMode: Binding.RestoreNone
                    }
                }
                Text { text: qtlottie.endFrame }
            }
        }

        GroupBox {
            id: aGroupBox
            title: "Frame Rate"
            contentItem: RowLayout {
                Slider {
                    enabled: (qtlottie.status === LottieAnimation.Ready)
                    from: 1
                    to: 100
                    stepSize: 1
                    value: qtlottie.frameRate
                    onValueChanged: { qtlottie.frameRate = value; }
                }
                Text { text: qtlottie.frameRate }
            }
        }

        GroupBox {
            title: "Scale"
            contentItem: RowLayout {
                Slider {
                    id: scaleSlider
                    enabled: (qtlottie.status === LottieAnimation.Ready)
                    from: 1
                    to: 200
                    stepSize: 1
                    value: 100
                    onValueChanged: { qtlottie.scale = value / 100; }
                }
                Text { text: qtlottie.scale.toFixed(2) }
            }
        }

        GroupBox {
            title: "Markers"
            enabled: qtlottie.markers.length > 0 && qtlottie.status === LottieAnimation.Ready
            contentItem: ComboBox {
                id: markerCombo
                model: qtlottie.markers
                onActivated: {
                    if (playButton.checked)
                        qtlottie.gotoAndStop(currentText);
                    else
                        qtlottie.gotoAndPlay(currentText);
                }
            }
        }

        GroupBox {
            title: "Status"
            contentItem: Label {
                text: (qtlottie.status === LottieAnimation.Null) ? "Null" :
                      (qtlottie.status === LottieAnimation.Loading) ? "Loading" :
                      (qtlottie.status === LottieAnimation.Ready) ? "Ready" :
                      (qtlottie.status === LottieAnimation.Error) ? "Error" : "Unknown"
                color: (qtlottie.status === LottieAnimation.Ready) ? "green" :
                       (qtlottie.status === LottieAnimation.Error) ? "red" : "yellow"
            }
        }
    }
}
