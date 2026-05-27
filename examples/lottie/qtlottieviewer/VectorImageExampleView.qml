// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs
import Qt.labs.lottieqt
import QtQuick.VectorImage

ColumnLayout {
    id: top
    spacing: 10
    property string source: ""

    Rectangle {
        id: canvas
        color: "lightgray"
        border.color: "black"
        border.width: 2
        radius: 10
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        Rectangle {
            id: vectorImgBackground
            anchors.centerIn: parent
            width: vectorImg.implicitWidth
            height: vectorImg.implicitHeight
            color: "#F0F0F0"

            VectorImage {
                id: vectorImg
                property LottieVectorImageController controller: LottieVectorImageController {}
                source: top.source
                assumeTrustedSource: true
                anchors.centerIn: parent
                preferredRendererType: VectorImage.CurveRenderer
                animations.loops: Animation.Infinite
                asynchronous: true
                asynchronousShapes: true

                onImplicitHeightChanged: {
                    if (implicitWidth > 0 && implicitHeight > 0) {
                        let newScale = Math.min(canvas.width / (implicitWidth * 1.2),
                                                canvas.height / (implicitHeight * 1.2))
                        scaleSlider.from = newScale * 10
                        scaleSlider.to = newScale * 400
                        scaleSlider.value = newScale * 100
                    }
                }
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            visible: running
            running: vectorImg.status === VectorImage.Loading
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
            enabled: (vectorImg.status === VectorImage.Ready)
            checkable: true
            text: checked ? "Play" : "Pause"
            onCheckedChanged: { vectorImg.controller.togglePause() }
            Layout.preferredHeight: aGroupBox.height
        }

        GroupBox {
            id: frameBox
            title: "Current Frame: " + vectorImg.controller.currentFrame.toFixed(1)
            enabled: playButton.checked && (vectorImg.status === VectorImage.Ready)
            contentItem: RowLayout {
                Text { text: vectorImg.controller.startFrame }
                Slider {
                    id: gotoSlider
                    from: vectorImg.controller.startFrame
                    to: vectorImg.controller.endFrame
                    stepSize: 1
                    value: 0
                    onValueChanged: { if (frameBox.enabled) vectorImg.controller.gotoAndStop(value) }
                    Binding on value {
                        when: !frameBox.enabled
                        value: vectorImg.controller.currentFrame
                        restoreMode: Binding.RestoreNone
                    }
                }
                Text { text: vectorImg.controller.endFrame }
            }
        }

        GroupBox {
            id: aGroupBox
            title: "Frame Rate"
            contentItem: RowLayout {
                Slider {
                    enabled: (vectorImg.status === VectorImage.Ready)
                    from: 1
                    to: 100
                    stepSize: 1
                    value: vectorImg.controller.frameRate
                    onValueChanged: { vectorImg.controller.frameRate = value }
                }
                Text { text: vectorImg.controller.frameRate }
            }
        }

        GroupBox {
            title: "Scale"
            contentItem: RowLayout {
                Slider {
                    id: scaleSlider
                    enabled: (vectorImg.status === VectorImage.Ready)
                    from: 1
                    to: 200
                    stepSize: 1
                    value: 100
                    onValueChanged: { vectorImgBackground.scale = value / 100 }
                }
                Text { text: vectorImgBackground.scale.toFixed(2) }
            }
        }

        GroupBox {
            title: "Status"
            contentItem: Label {
                text: (vectorImg.status === VectorImage.Null) ? "Null" :
                      (vectorImg.status === VectorImage.Loading) ? "Loading" :
                      (vectorImg.status === VectorImage.Ready) ? "Ready" :
                      (vectorImg.status === VectorImage.Error) ? "Error" : "Unknown"
                color: (vectorImg.status === VectorImage.Ready) ? "green" :
                       (vectorImg.status === VectorImage.Error) ? "red" : "yellow"
            }
        }
    }
}
