
# Mike Wallbank
# February 2016

install_fhicl()
install_headers()
install_source()

set(${product}_file_dir "${product}/${version}/files")
file(GLOB channel_map_file 35tTPCChannelMap_v*.txt)
install(FILES ${channel_map_file} DESTINATION ${${product}_file_dir})

# Add plugin for each service
simple_plugin(ChannelMapService "service"
	      ${ART_PERSISTENCY_ROOTDB}
	      ${ART_FRAMEWORK_SERVICES_REGISTRY}
	      ${ART_UTILITIES}
	      ${SQLITE3}
	      ${MF_MESSAGELOGGER}
	      ${MF_UTILITIES}
	      ${CETLIB}
	      ${ROOT_BASIC_LIB_LIST}
)

