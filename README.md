# CTH++

## Introduction

### What is CTH++

## Usage

```text
USAGE: cth++ [options]
OPTIONS:
Generic Options:
  --help                                - Display available options (--help-hidden for more)
  --help-list                           - Display list of available options (--help-list-hidden for more)
  --version                             - Display the version of this program
cth++ options:
  --cmake-target-current-build=<target> - Specify the current build target
  --config=<path>                       - Path to the JSON configuration file
  --dbg                                 - Set build mode to debug
  --development                         - Set build mode to development
  --namespace=<name>                    - Set the global namespace name
  --output=<path>                       - Output path
  --prodaction                          - Set build mode to production
  --rel                                 - Set build mode to release
  --std=<cxx standard>                  - Specify the C++ standard
  --working=<path>                      - Set the project directory
```

# Example

```shell
$ cth++ --output=path/to/output --config=path/to/config.json
```

- config.json

```json

{
	"project": {
		"name": "Example",
		"desc": "Example Example Example",
		"version": "1.1.1",
		"debug": true,
		// --dbg true / --rel false
		"dev": true,
		// --development / --prodaction
		"output-path": "path/to/output",
		// ignored use override argumend --output
		"project-dir": "path/to/project/dir"
		// argument --working
	},
	"config": {
		"client": {
			"port": 52485,
			"host": "localhost"
		},
		"server": {
			"port": 2000,
			"host": "localhost",
			"options": {
				"max-connections": 10,
				"offline": false,
				"debug": {
					"max-connections": 5
				}
			}
		},
		"master": {
			"port": 2000,
			"host": "localhost"
		}
	}
}
```

- config.hpp

```c++
// path/to/output
/*
  _____                                      _        
 | ____| __  __   __ _   _ __ ___    _ __   | |   ___ 
 |  _|   \ \/ /  / _` | | '_ ` _ \  | '_ \  | |  / _ \
 | |___   >  <  | (_| | | | | | | | | |_) | | | |  __/
 |_____| /_/\_\  \__,_| |_| |_| |_| | .__/  |_|  \___|
                                    |_|               
*/

#pragma once

#define VERSION_PACK(MAJOR, MINOR, PATCH) \
    ( ( ( MAJOR ) << 16 ) | ( ( MINOR ) << 8 ) | ( PATCH ) )


namespace config {
    namespace project {
        constexpr char *name = "Example";
        constexpr char *description = "Example work with config";
        constexpr char *git_hash = "fb3483f";
        constexpr unsigned int version = 65536;
        constexpr bool debug = true;
        constexpr bool release = false;
        constexpr bool development = true;
        constexpr bool production = false;
        constexpr char *target = "";
        constexpr char *mode = "development";
        constexpr char *type = "debug";
    }
    namespace client {
        constexpr char *host = "localhost";
        constexpr unsigned long long port = 52485ULL;
    }
    namespace master {
        constexpr char *host = "localhost";
        constexpr unsigned long long port = 2000ULL;
    }
    namespace server {
        constexpr char *host = "localhost";
        namespace options {
            namespace debug {
                constexpr unsigned long long max_connections = 5ULL;
            }
            constexpr unsigned long long max_connections = 10ULL;
            constexpr bool offline = false;
        }
        constexpr unsigned long long port = 2000ULL;
    }
}
```