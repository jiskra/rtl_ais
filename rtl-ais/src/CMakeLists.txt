# Copyright 2012 OSMOCOM Project
#
# This file is part of rtl-sdr
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.


# ########################################################################
# # Install built library files & utilities
# ########################################################################
# )
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src 
  ${CMAKE_SOURCE_DIR}/src/convenience ${CMAKE_SOURCE_DIR}/src/aisdecoder
  ${CMAKE_SOURCE_DIR}/src/aisdecoder/lib)


SET(rtlsdr_shared 
    librtlsdr.c
    tuner_e4k.c
    tuner_fc0012.c
    tuner_fc0013.c
    tuner_fc2580.c
    tuner_r82xx.c
    )

add_executable (rtl_ais
    convenience/convenience.c
    librtlsdr.c
    tuner_e4k.c
    tuner_fc0012.c
    tuner_fc0013.c
    tuner_fc2580.c
    tuner_r82xx.c
    aisdecoder/aisdecoder.c
    aisdecoder/sounddecoder.c
    aisdecoder/lib/receiver.c
    aisdecoder/lib/protodec.c
    aisdecoder/lib/hmalloc.c
    aisdecoder/lib/filter.c
    rtl_ais.c
    )
add_executable(rtl_test
    convenience/convenience.c
    librtlsdr.c
    tuner_e4k.c
    tuner_fc0012.c
    tuner_fc0013.c
    tuner_fc2580.c
    tuner_r82xx.c
    rtl_test.c
    )
    
target_link_libraries(rtl_ais  convenience_static
    ${LIBUSB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
# target_link_libraries(rtl_ais
#     ${LIBUSB_LIBRARIES}
# )
target_link_libraries(rtl_test  convenience_static
    ${LIBUSB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)
# target_link_libraries(rtl_test
#     ${LIBUSB_LIBRARIES}
# )
if(UNIX)
    target_link_libraries(rtl_ais m)
    if(APPLE)
    target_link_libraries(rtl_test m)
else()
    target_link_libraries(rtl_test m rt)
endif()
endif()

if(WIN32)
#    target_link_libraries(rtl_fm libgetopt_static)
#    target_link_libraries(rtl_fm libgetopt_static)
    set_property(TARGET rtl_ais APPEND PROPERTY COMPILE_DEFINITIONS "rtlsdr_STATIC" )
    set_property(TARGET rtl_test APPEND PROPERTY COMPILE_DEFINITIONS "rtlsdr_STATIC" )
endif()
