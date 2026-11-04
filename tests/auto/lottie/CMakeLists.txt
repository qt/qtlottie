add_subdirectory(shape)
if(QT_FEATURE_private_tests)
    add_subdirectory(parse)
endif()

if(TARGET Qt::QuickPrivate
   AND TARGET Qt::QuickShapesPrivate
   AND TARGET Qt::QuickVectorImageGeneratorPrivate)
    add_subdirectory(vectorimage)
endif()
