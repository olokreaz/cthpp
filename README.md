# Compile Time generator of configs
---
GCT - это генератор CT конфига, тоесть из простого конфига, сгенерируется config.h / config.hpp файл, в котором будет все сгенерированые 
---

#### Usage: 
```hjson
{
  project: {
    name: my_project
    version: 1.0.0
    type: debug|developing
  }
  config: {
    server: {
      log: {
        level: {
          debug: TRACE
          release: WARN
        }
      }
    }
  }
}
```

```cpp
...
namespace my_project{
  namespace project{
    constexpr char* name = u8"my_project";
    constexpr char* version = u8"1.0.0";
    constexpr bool debug = true;
    constexpr bool developing = true;
    constepxr bool release = false;
    constexpr bool production = false;
  }
  namespacee config {
    namespace server {
      namespacec log {
        constexpr char* level = u8"TRACE";
      }
    }
  }
}
...
```


### reference
- buildroot config(make nconfig)
- linux dialog
