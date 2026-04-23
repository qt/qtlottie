// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

ApplicationWindow {
    id: appWindow
    visible: true
    width: 1280
    height: 960
    property string lottieFile: ""
    title: lottieFile + " - QtLottie Viewer"

    Component.onCompleted: {
        if (Qt.application.arguments.length > 1)
            lottieFile = Qt.resolvedUrl("file:" + Qt.application.arguments[1]);
        else
            lottieFile = "qrc:/qtlottieviewer/default_file.json";
    }

    TabBar {
        id: bar
        width: parent.width - 20
        anchors.horizontalCenter: parent.horizontalCenter

        TabButton {
            text: qsTr("VectorImage (QuickShapes GPU rendering)")
        }

        TabButton {
            text: qsTr("LottieAnimation (QPainter CPU rendering)")
        }
    }

    Item {
        anchors.fill: parent
        anchors.topMargin: bar.height + 4
        anchors.margins: 10

        VectorImageExampleView {
            visible: bar.currentIndex === 0
            anchors.fill: parent
            source: appWindow.lottieFile
        }

        LottieAnimationExampleView {
            visible: bar.currentIndex === 1
            anchors.fill: parent
            source: appWindow.lottieFile
        }
    }

    FileDialog {
        id: fileDialog
        title: "Select a Lottie file"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Lottie files (*.json)", "All files (*)"]
        onAccepted: {
            appWindow.lottieFile = fileDialog.selectedFile;
            fileDialog.visible = false;
        }
    }
}
