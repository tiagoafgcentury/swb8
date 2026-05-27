function(add_efl TARGET)
    target_include_directories(${TARGET} PUBLIC
        /opt/ali/efl/sysroot/usr/include/efl-1/
        /opt/ali/efl/sysroot/usr/include/eina-1/
        /opt/ali/efl/sysroot/usr/include/eina-1/eina/
        /opt/ali/efl/sysroot/usr/include/emile-1/
        /opt/ali/efl/sysroot/usr/include/eo-1/
        /opt/ali/efl/sysroot/usr/include/evas-1/
        /opt/ali/efl/sysroot/usr/include/ecore-1/
        /opt/ali/efl/sysroot/usr/include/ecore-evas-1/
    )

    target_link_directories(${TARGET} PUBLIC /opt/ali/efl/sysroot/usr/lib/)

    target_link_libraries(${TARGET} PUBLIC ecore_evas efl eo eina evas eet ecore emile efl eina ector freetype luajit-5.1 eet ssl crypto lz4 z pthread m rt dl)
endfunction()
