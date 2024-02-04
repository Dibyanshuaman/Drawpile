#[[
This module defines versions and other similar constants that are used
throughout the project.
#]]

set(DRAWPILE_PROTO_DEFAULT_PORT 27750)
set(DRAWPILE_WEBADMIN_DEFAULT_PORT 27780)
set(DRAWPILE_WEBSOCKET_DEFAULT_PORT 27751)

set(semver_regexp "([0-9]+\\.[0-9]+\\.[0-9]+)((-[A-Za-z0-9.-]*)?(\\+[A-Za-z0-9.-]*)?)")

include(GitVersion)
git_version_describe(dp_version)
if(dp_version MATCHES "^v?${semver_regexp}")
	set(PROJECT_VERSION ${CMAKE_MATCH_1})
	set(PROJECT_VERSION_LABEL ${CMAKE_MATCH_2})
else()
	file(STRINGS Cargo.toml dp_version LIMIT_COUNT 1 REGEX "^version[ \t]*=[ \t]*\"[^\"]*\"$")
	if(dp_version MATCHES "^version[ \t]*=[ \t]*\"${semver_regexp}\"")
		set(PROJECT_VERSION ${CMAKE_MATCH_1})
		set(PROJECT_VERSION_LABEL ${CMAKE_MATCH_2})
	else()
		message(FATAL_ERROR "Invalid version in Cargo.toml")
	endif()
endif()
unset(dp_version)

set(dp_proto_regex "^[ \t]*version:[ \t]*\"?dp:([0-9]+)\\.([0-9]+)\\.([0-9]+)\"?$")
file(STRINGS src/drawdance/generators/protogen/protocol.yaml dp_proto_version LIMIT_COUNT 1 REGEX ${dp_proto_regex})
if(dp_proto_version MATCHES ${dp_proto_regex})
	set(DRAWPILE_PROTO_SERVER_VERSION ${CMAKE_MATCH_1})
	set(DRAWPILE_PROTO_MAJOR_VERSION ${CMAKE_MATCH_2})
	set(DRAWPILE_PROTO_MINOR_VERSION ${CMAKE_MATCH_3})
else()
	message(FATAL_ERROR "Invalid protocol.yaml")
endif()

unset(semver_regexp)
