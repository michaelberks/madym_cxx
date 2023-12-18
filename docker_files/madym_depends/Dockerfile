FROM registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_depends_no_gui

#Add the QT stuff required my the Madym GUI and package builder
COPY Qt Qt
COPY cqtdeployer cqtdeployer
RUN ln -s /cqtdeployer/cqtdeployer.sh /usr/bin/cqtdeployer \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /Qt/5.15.2/gcc_64/lib/libicuuc.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /Qt/5.15.2/gcc_64/lib/libicudata.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /Qt/5.15.2/gcc_64/lib/libicui18n.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /cqtdeployer/lib/libicuuc.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /cqtdeployer/lib/libicudata.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /cqtdeployer/lib/libicui18n.so.56
    
ENV Qt5Core_DIR /Qt/5.15.2/gcc_64/lib/cmake/Qt5core
ENV Qt5Gui_DIR /Qt/5.15.2/gcc_64/lib/cmake/Qt5Gui
ENV Qt5Widgets_DIR /Qt/5.15.2/gcc_64/lib/cmake/Qt5Widgets   