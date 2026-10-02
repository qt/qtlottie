// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Window
import Qt.labs.lottieqt
import QtQuick.VectorImage

Rectangle {
    id: toplevel
    width: 800 / Screen.devicePixelRatio
    height: width
    color: "darkgray"
    property real dim: (width / 2) - grid.spacing

    Grid {
        id: grid
        columns: 2
        padding: spacing / 2
        spacing: toplevel.width / 80

        Repeater {
            model: 2

            Image {
                source: "qrc:///checkered.png"
                fillMode: Image.Tile
                width: toplevel.dim
                height: width
                horizontalAlignment: Image.AlignLeft
                verticalAlignment: Image.AlignTop

                VectorImage {
                    source: qtlottie.source
                    assumeTrustedSource: true
                    property bool _qt_usenondefaultgenerator: (index === 1)
                    anchors.fill: parent
                    fillMode: VectorImage.PreserveAspectFit

                    preferredRendererType: VectorImage.CurveRenderer
                    animations.paused: true
                    clip: true
                }
            }
        }

        Image {
            source: "qrc:///checkered.png"
            fillMode: Image.Tile
            width: toplevel.dim
            height: width
            horizontalAlignment: Image.AlignLeft
            verticalAlignment: Image.AlignTop

            LottieAnimation {
                id: qtlottie
                scale: Math.min(toplevel.dim / width, toplevel.dim / height)
                transformOrigin: Item.TopLeft
                textureSize: Qt.size(toplevel.dim, toplevel.dim)
                objectName: "qtlottie_animation_item"
                quality: LottieAnimation.HighQuality
                autoPlay: false
                property int freezeFrame: -1
                onStatusChanged: {
                    if (status === LottieAnimation.Ready) {
                        if (freezeFrame < 0)
                            freezeFrame = Math.floor(startFrame + ((endFrame - startFrame) / 2));
                        gotoAndStop(freezeFrame);
                    }
                }
            }
        }
    }
}
