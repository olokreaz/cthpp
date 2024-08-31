# CTH++

## Introduction

Config Compile-Time Header++ (CTH++) is a tool for automatically generating C++ configuration headers at compile-time
using JSON data and constexpr. It supports various build modes and offers flexible project configuration.

## Usage

```text
USAGE: cth++.exe [options]

OPTIONS:

Generic Options:

  --help                                - Display available options (--help-hidden for more)
  --help-list                           - Display list of available options (--help-list-hidden for more)
  --version                             - Display the version of this program

cth++ options:

  --cmake-target-current-build=<target> - Specify the current build target
  --config=<path>                       - Path to the JSON configuration file
  --dbg                                 - Set build mode to debug
  --dev                                 - Set build mode to development
  --namespace=<name>                    - Set the global namespace name
  --no-git                              - Disable git hash
  --no-logo                             - Disable logo
  --output=<path>                       - Output path
  --prod                                - Set build mode to production
  --rel                                 - Set build mode to release
  --rewrite-config                      - Rewrite the configuration file
  --std=<cxx standard>                  - Specify the C++ standard
  --target-arch=<arch>                  - Specify the target architecture
  --target-system=<system>              - Specify the target system
  --working-dir=<path>                  - Set the project directory
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
