qt_internal_add_qml_module(lottieqtplugin
    URI "Qt.labs.lottieqt"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME BodymovinPlugin
    PLUGIN_TARGET lottieqtplugin
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    NO_GENERATE_QMLTYPES
    INSTALL_SOURCE_QMLTYPES "plugins.qmltypes"
    SOURCES
        lottie_plugin.cpp lottie_plugin.h
        lottieanimation.cpp lottieanimation.h
        rasterrenderer/batchrenderer.cpp rasterrenderer/batchrenderer.h
        rasterrenderer/lottierasterrenderer.cpp rasterrenderer/lottierasterrenderer.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::GuiPrivate
        Qt::Qml
        Qt::Quick
        Qt::BodymovinPrivate
)

qt_internal_add_docs(lottieqtplugin
    doc/qtlottieanimation.qdocconf
)
