# Changelog

## [1.7.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.7.0)

### Features

- Support user defined allocators ([88162d1f](https://github.com/espressif/esp-protocols/commit/88162d1f))
- Allow allocate memory with configured caps ([7d29b476](https://github.com/espressif/esp-protocols/commit/7d29b476))

### Bug Fixes

- Adjust some formatting per indent-cont=120 ([5b2077e3](https://github.com/espressif/esp-protocols/commit/5b2077e3))

## [1.6.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.6.0)

### Features

- support allocating mDNS task from SPIRAM ([8fcad10c](https://github.com/espressif/esp-protocols/commit/8fcad10c))

### Bug Fixes

- Use correct task delete function ([eb4ab524](https://github.com/espressif/esp-protocols/commit/eb4ab524))

### Updated

- ci(mdns): Fix mdns host test layers with static task creation ([0690eba3](https://github.com/espressif/esp-protocols/commit/0690eba3))

## [1.5.3](https://github.com/espressif/esp-protocols/commits/mdns-v1.5.3)

### Bug Fixes

- Fix responder to ignore only invalid queries ([cd07228f](https://github.com/espressif/esp-protocols/commit/cd07228f), [#754](https://github.com/espressif/esp-protocols/issues/754))

## [1.5.2](https://github.com/espressif/esp-protocols/commits/mdns-v1.5.2)

### Bug Fixes

- Fix potential NULL deref when sending sub-buy ([e7273c46](https://github.com/espressif/esp-protocols/commit/e7273c46))
- Fix _mdns_append_fqdn excessive stack usage ([bd23c233](https://github.com/espressif/esp-protocols/commit/bd23c233))

## [1.5.1](https://github.com/espressif/esp-protocols/commits/mdns-v1.5.1)

### Bug Fixes

- Fix incorrect memory free for mdns browse ([4451a8c5](https://github.com/espressif/esp-protocols/commit/4451a8c5))

## [1.5.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.5.0)

### Features

- supported removal of subtype when updating service ([4ad88e29](https://github.com/espressif/esp-protocols/commit/4ad88e29))

### Bug Fixes

- Fix zero-sized VLA clang-tidy warnings ([196198ec](https://github.com/espressif/esp-protocols/commit/196198ec))
- Remove dead store to arg variable shared ([e838bf03](https://github.com/espressif/esp-protocols/commit/e838bf03))
- Fix name mangling not to use strcpy() ([99b54ac3](https://github.com/espressif/esp-protocols/commit/99b54ac3))
- Fix potential null derefernce in _mdns_execute_action() ([f5be2f41](https://github.com/espressif/esp-protocols/commit/f5be2f41))
- Fix AFL test mock per espressif/esp-idf@a5bc08fb55c ([3d8835cf](https://github.com/espressif/esp-protocols/commit/3d8835cf))
- Fixed potential out-of-bound interface error ([24f55ce9](https://github.com/espressif/esp-protocols/commit/24f55ce9))
- Fixed incorrect error conversion ([8f8516cc](https://github.com/espressif/esp-protocols/commit/8f8516cc))
- Fixed potential overflow when allocating txt data ([75a8e864](https://github.com/espressif/esp-protocols/commit/75a8e864))
- Move MDNS_NAME_BUF_LEN to public headers ([907087c0](https://github.com/espressif/esp-protocols/commit/907087c0), [#724](https://github.com/espressif/esp-protocols/issues/724))
- Cleanup includes in mdns.c ([68a9e148](https://github.com/espressif/esp-protocols/commit/68a9e148), [#725](https://github.com/espressif/esp-protocols/issues/725))
- Allow advertizing service with port==0 ([827ea65f](https://github.com/espressif/esp-protocols/commit/827ea65f))
- Fixed complier warning if MDNS_MAX_SERVICES==0 ([95377216](https://github.com/espressif/esp-protocols/commit/95377216), [#611](https://github.com/espressif/esp-protocols/issues/611))

## [1.4.3](https://github.com/espressif/esp-protocols/commits/mdns-v1.4.3)

### Features

- support zero item when update subtype ([5bd82c01](https://github.com/espressif/esp-protocols/commit/5bd82c01))

## [1.4.2](https://github.com/espressif/esp-protocols/commits/mdns-v1.4.2)

### Features

- support update subtype ([062b8dca](https://github.com/espressif/esp-protocols/commit/062b8dca))

### Updated

- chore(mdns): Add more info to idf_component.yml ([4a1cb65c](https://github.com/espressif/esp-protocols/commit/4a1cb65c))

## [1.4.1](https://github.com/espressif/esp-protocols/commits/mdns-v1.4.1)

### Features

- Send PTR query for mdns browse when interface is ready ([010a404a](https://github.com/espressif/esp-protocols/commit/010a404a))

### Bug Fixes

- Prevent deadlock when deleting a browse request ([3f48f9ea](https://github.com/espressif/esp-protocols/commit/3f48f9ea))
- Fix use after free reported by coverity ([25b3d5fd](https://github.com/espressif/esp-protocols/commit/25b3d5fd))
- Fixed dead-code reported by coverity ([11846c7d](https://github.com/espressif/esp-protocols/commit/11846c7d))

## [1.4.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.4.0)

### Major changes

- Fixed mdns API issues when add/remove/update records from multiple threads ([Fix services API races to directly add/remove services](https://github.com/espressif/esp-protocols/commit/8a690503))

### Features

- Unit tests for add/remove/update deleg/selfhosted services ([0660ece1](https://github.com/espressif/esp-protocols/commit/0660ece1))
- Add console command for mdns browsing ([1e8ede33](https://github.com/espressif/esp-protocols/commit/1e8ede33))
- Console test: set instance for service ([f107dcd1](https://github.com/espressif/esp-protocols/commit/f107dcd1))
- Console test: add subtype for service ([ee00e97b](https://github.com/espressif/esp-protocols/commit/ee00e97b))
- Console test: set port for (delegated) srvs ([07b79abf](https://github.com/espressif/esp-protocols/commit/07b79abf))
- Console test: add/remove TXT recs for delegated srvs ([c9a58d73](https://github.com/espressif/esp-protocols/commit/c9a58d73))
- Console test for changing TXT records ([6b9a6ce6](https://github.com/espressif/esp-protocols/commit/6b9a6ce6))
- Console test for add/remove delegated service APIs ([43de7e5c](https://github.com/espressif/esp-protocols/commit/43de7e5c))
- Console test for add/remove delegated host APIs ([ce7f326a](https://github.com/espressif/esp-protocols/commit/ce7f326a))
- Console test for lookup service APIs ([a91ead8e](https://github.com/espressif/esp-protocols/commit/a91ead8e))
- Add linux console functional tests ([50d059af](https://github.com/espressif/esp-protocols/commit/50d059af))
- check if the txt items is changed when browsing ([e2f0477a](https://github.com/espressif/esp-protocols/commit/e2f0477a))

### Bug Fixes

- Fix mdns_delegate_hostname_add() to block until done ([2c1b1661](https://github.com/espressif/esp-protocols/commit/2c1b1661))
- Fix API races when removing all services ([169405b5](https://github.com/espressif/esp-protocols/commit/169405b5))
- Fix API races setting instance name for services ([643dc6d4](https://github.com/espressif/esp-protocols/commit/643dc6d4))
- Fix API races while adding subtypes for services ([f9f234c4](https://github.com/espressif/esp-protocols/commit/f9f234c4))
- Fix API races removing txt item for services ([3f97a822](https://github.com/espressif/esp-protocols/commit/3f97a822))
- Fix API races adding txt item for services ([c62b920b](https://github.com/espressif/esp-protocols/commit/c62b920b))
- Fix API races while setting txt for services ([a927bf3a](https://github.com/espressif/esp-protocols/commit/a927bf3a))
- Fix API races while setting port for services ([99d5fb27](https://github.com/espressif/esp-protocols/commit/99d5fb27))
- Fix services API races to directly add/remove services ([8a690503](https://github.com/espressif/esp-protocols/commit/8a690503))
- Fix mdns mdns_lookup_service() to handle empty TXT ([d4da9cb0](https://github.com/espressif/esp-protocols/commit/d4da9cb0))

## [1.3.2](https://github.com/espressif/esp-protocols/commits/mdns-v1.3.2)

### Features

- add check of instance when handling PTR query ([6af6ca5](https://github.com/espressif/esp-protocols/commit/6af6ca5))

### Bug Fixes

- Fix of mdns afl tests ([139166c](https://github.com/espressif/esp-protocols/commit/139166c))
- remove same protocol services with different instances ([042533a](https://github.com/espressif/esp-protocols/commit/042533a))

## [1.3.1](https://github.com/espressif/esp-protocols/commits/mdns-v1.3.1)

### Bug Fixes

- free txt value len ([afd98bb](https://github.com/espressif/esp-protocols/commit/afd98bb))

## [1.3.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.3.0)

### Features

- add a new mdns query mode `browse` ([af330b6](https://github.com/espressif/esp-protocols/commit/af330b6))
- Make including mdns_console KConfigurable ([27adbfe](https://github.com/espressif/esp-protocols/commit/27adbfe))

### Bug Fixes

- Schedule all queued Tx packets from timer task ([d4e693e](https://github.com/espressif/esp-protocols/commit/d4e693e))
- add lock for some common apis ([21c84bf](https://github.com/espressif/esp-protocols/commit/21c84bf))
- fix mdns answer append while host is invalid ([7be16bc](https://github.com/espressif/esp-protocols/commit/7be16bc))

## [1.2.5](https://github.com/espressif/esp-protocols/commits/mdns-v1.2.5)

### Bug Fixes

- Fixed build issues for targets without WiFi caps ([302b46f](https://github.com/espressif/esp-protocols/commit/302b46f))

## [1.2.4](https://github.com/espressif/esp-protocols/commits/mdns-v1.2.4)

### Bug Fixes

- Correction on 6d2c475 MDNS_PREDEF_NETIF_ETH fix ([fc59f87c4e](https://github.com/espressif/esp-protocols/commit/fc59f87c4e))
- fix the logic of creating pcb for networking socket ([5000a9a20a](https://github.com/espressif/esp-protocols/commit/5000a9a20a))
- fix compiling issue when disabling IPv4 ([2646dcd23a](https://github.com/espressif/esp-protocols/commit/2646dcd23a))
- Fix compile error when MDNS_PREDEF_NETIF_ETH is defined, but ETH_ENABLED is not (#459) ([6d2c475c20](https://github.com/espressif/esp-protocols/commit/6d2c475c20))

## [1.2.3](https://github.com/espressif/esp-protocols/commits/mdns-v1.2.3)

### Bug Fixes

- fixed CI issues for host and afl tests ([4be5efc84e](https://github.com/espressif/esp-protocols/commit/4be5efc84e))
- fix copy delegated host addr ([19fb36000c](https://github.com/espressif/esp-protocols/commit/19fb36000c))
- enable CONFIG_ESP_WIFI_ENABLED if CONFIG_SOC_WIFI_SUPPORTED is also enabled (for ESP-IDF <= 5.1) ([d20a718320](https://github.com/espressif/esp-protocols/commit/d20a718320))
- remove protocol_examples_common ([1ee9dae6bf](https://github.com/espressif/esp-protocols/commit/1ee9dae6bf))
- move the example into a subdirectory ([d28232b9f8](https://github.com/espressif/esp-protocols/commit/d28232b9f8))
- reference protocol_examples_common from IDF ([c83b76ea8f](https://github.com/espressif/esp-protocols/commit/c83b76ea8f))

## [1.2.2](https://github.com/espressif/esp-protocols/commits/mdns-v1.2.2)

### Bug Fixes

- add terminator for the getting host name ([b6a4d94](https://github.com/espressif/esp-protocols/commit/b6a4d94))
- Enable ESP_WIFI_CONFIG when ESP-IDF <= 5.1 ([0b783c0](https://github.com/espressif/esp-protocols/commit/0b783c0))
- set host list NULL on destroy ([ea54eef](https://github.com/espressif/esp-protocols/commit/ea54eef))
- removed Wno-format flag and fixed formatting warnings ([c48e442](https://github.com/espressif/esp-protocols/commit/c48e442))
- remove the the range of MDNS_MAX_SERVICES and fix issues of string functions ([3dadce2](https://github.com/espressif/esp-protocols/commit/3dadce2))

## [1.2.1](https://github.com/espressif/esp-protocols/commits/mdns-v1.2.1)

### Features

- Allow setting length of mDNS action queue in menuconfig ([28cd898](https://github.com/espressif/esp-protocols/commit/28cd898))

### Bug Fixes

- fix build issue if CONFIG_ESP_WIFI_ENABLED disabled ([24f7031](https://github.com/espressif/esp-protocols/commit/24f7031))
- added idf_component.yml for examples ([d273e10](https://github.com/espressif/esp-protocols/commit/d273e10))
- added guard check for null pointer ([71bb461](https://github.com/espressif/esp-protocols/commit/71bb461))

## [1.2.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.2.0)

### Features

- add an API for setting address to a delegated host ([ddc3eb6](https://github.com/espressif/esp-protocols/commit/ddc3eb6))
- Add support for lwip build under linux ([588465d](https://github.com/espressif/esp-protocols/commit/588465d))
- Allow for adding a delegated host with no address ([c562461](https://github.com/espressif/esp-protocols/commit/c562461))
- Add APIs for looking up self hosted services and getting the self hostname ([f0df12d](https://github.com/espressif/esp-protocols/commit/f0df12d))

### Bug Fixes

- Refactor freertos linux compat layers ([79a0e57](https://github.com/espressif/esp-protocols/commit/79a0e57))
- Fix delegated service PTR response ([cab0e1d](https://github.com/espressif/esp-protocols/commit/cab0e1d))
- Added unit tests to CI + minor fix to pass it ([c974c14](https://github.com/espressif/esp-protocols/commit/c974c14))

### Updated

- docs: update documentation links ([4de5298](https://github.com/espressif/esp-protocols/commit/4de5298))

## [1.1.0](https://github.com/espressif/esp-protocols/commits/mdns-v1.1.0)

### Features

- Decouple main module from mdns-networking ([d238e93](https://github.com/espressif/esp-protocols/commit/d238e93))

### Bug Fixes

- Use idf-build-apps package for building mdns ([1a0a41f](https://github.com/espressif/esp-protocols/commit/1a0a41f))
- socket networking to init interfaces properly ([ee9b04f](https://github.com/espressif/esp-protocols/commit/ee9b04f))
- Removed unused internal lock from mdns_server struct ([a06fb77](https://github.com/espressif/esp-protocols/commit/a06fb77))
- Resolve conflicts only on self hosted items ([e69a9eb](https://github.com/espressif/esp-protocols/commit/e69a9eb), [#185](https://github.com/espressif/esp-protocols/issues/185))
- Fix memory issues reported by valgrind ([0a682e7](https://github.com/espressif/esp-protocols/commit/0a682e7))

### Updated

- docs(common): updated component and example links ([f48d9b2](https://github.com/espressif/esp-protocols/commit/f48d9b2))
- Add APIs to look up delegated services ([87dcd7d](https://github.com/espressif/esp-protocols/commit/87dcd7d))
- Fix deadly mdns crash ([4fa3023](https://github.com/espressif/esp-protocols/commit/4fa3023))
- docs(common): improving documentation ([ca3fce0](https://github.com/espressif/esp-protocols/commit/ca3fce0))
- append all ipv6 address in mdns answer ([5ed3e9a](https://github.com/espressif/esp-protocols/commit/5ed3e9a))
- test(mdns): Host tests to use IDF's esp_event_stub ([537d170](https://github.com/espressif/esp-protocols/commit/537d170))

## [1.0.9](https://github.com/espressif/esp-protocols/commits/mdns-v1.0.9)

### Features

- Add reverse lookup to the example and test ([d464ed7](https://github.com/espressif/esp-protocols/commit/d464ed7))
- Add support for IPv6 reverse query ([d4825f5](https://github.com/espressif/esp-protocols/commit/d4825f5))

### Bug Fixes

- Reintroduce missing CHANGELOGs ([200cbb3](https://github.com/espressif/esp-protocols/commit/200cbb3))
- use semaphore instead of task notification bits (IDFGH-9380) ([73f2800](https://github.com/espressif/esp-protocols/commit/73f2800), [IDF#10754](https://github.com/espressif/esp-idf/issues/10754))

### Updated

- ci(common): force scoping commit messages with components ([c55fcc0](https://github.com/espressif/esp-protocols/commit/c55fcc0))
- Add homepage URL and License to all components ([ef3f0ee](https://github.com/espressif/esp-protocols/commit/ef3f0ee))
- docs: fix of mdns link translation ([1c850dd](https://github.com/espressif/esp-protocols/commit/1c850dd))
- unite all tags under common structure py test: update tags under common structure ([c6db3ea](https://github.com/espressif/esp-protocols/commit/c6db3ea))

## [1.0.8](https://github.com/espressif/esp-protocols/commits/b9b4a75)

### Features

- Add support for IPv4 reverse query ([b87bef5](https://github.com/espressif/esp-protocols/commit/b87bef5))

### Bug Fixes

- Host test with IDFv5.1 ([fb8a2f0](https://github.com/espressif/esp-protocols/commit/fb8a2f0))
- Remove strict mode as it's invalid ([d0c9070](https://github.com/espressif/esp-protocols/commit/d0c9070))
- Allow setting instance name only after hostname set ([a8339e4](https://github.com/espressif/esp-protocols/commit/a8339e4), [#190](https://github.com/espressif/esp-protocols/issues/190))
- Make unit test executable with pytest ([12cfcb5](https://github.com/espressif/esp-protocols/commit/12cfcb5))
- AFL port layer per IDF-latest changes ([0247926](https://github.com/espressif/esp-protocols/commit/0247926))

### Updated

- bump the component version to 1.0.8 ([b9b4a75](https://github.com/espressif/esp-protocols/commit/b9b4a75))
- Make reverse query conditional per Kconfig ([91134f1](https://github.com/espressif/esp-protocols/commit/91134f1))
- Added badges with version of components to the respective README files ([e4c8a59](https://github.com/espressif/esp-protocols/commit/e4c8a59))
- fix some coverity reported issues ([c73c797](https://github.com/espressif/esp-protocols/commit/c73c797))
- Examples: using pytest.ini from top level directory ([aee016d](https://github.com/espressif/esp-protocols/commit/aee016d))
- mDNS: test_app pytest migration ([f71f61f](https://github.com/espressif/esp-protocols/commit/f71f61f))
- CI: fixing the files to be complient with pre-commit hooks ([945bd17](https://github.com/espressif/esp-protocols/commit/945bd17))
- prevent crash when hostname is null ([3498e86](https://github.com/espressif/esp-protocols/commit/3498e86))
- Example tests integration ([a045c1c](https://github.com/espressif/esp-protocols/commit/a045c1c))
- Replace hardcoded TTL values with named defines ([bb4c002](https://github.com/espressif/esp-protocols/commit/bb4c002))
- Fix add_service() to report error if no-hostname ([656ab21](https://github.com/espressif/esp-protocols/commit/656ab21))


## [1.0.7](https://github.com/espressif/esp-protocols/commits/088f7ac)

### Updated

- bump up the component version ([088f7ac](https://github.com/espressif/esp-protocols/commit/088f7ac))
- fix IPV4 only build and also update CI configuration ([e079f8b](https://github.com/espressif/esp-protocols/commit/e079f8b))
- add test configuration for IPV6 disabled build ([330332a](https://github.com/espressif/esp-protocols/commit/330332a))


## [1.0.6](https://github.com/espressif/esp-protocols/commits/48c157b)

### Bug Fixes

- Example makefile to add only mdns as extra comps ([d74c296](https://github.com/espressif/esp-protocols/commit/d74c296))
- ignore authoritative flag on reception ([415e04a](https://github.com/espressif/esp-protocols/commit/415e04a))

### Updated

- fix build issue with CONFIG_LWIP_IPV6 disabled ([48c157b](https://github.com/espressif/esp-protocols/commit/48c157b))
- fix bit order issue in DNS header flags ([c4e85bd](https://github.com/espressif/esp-protocols/commit/c4e85bd))
- updated package version to 0.1.19 ([469f953](https://github.com/espressif/esp-protocols/commit/469f953))


## [1.0.5](https://github.com/espressif/esp-protocols/commits/36de9af)

### Features

- Define explicit dependencies on esp-wifi ([36de9af](https://github.com/espressif/esp-protocols/commit/36de9af))

### Updated

- bugfix: mdns IPv6 address convert error ([238ee96](https://github.com/espressif/esp-protocols/commit/238ee96))


## [1.0.4](https://github.com/espressif/esp-protocols/commits/57afa38)

### Updated

- Bump asio/mdns/esp_websocket_client versions ([57afa38](https://github.com/espressif/esp-protocols/commit/57afa38))
- ignore format warnings ([d66f9dc](https://github.com/espressif/esp-protocols/commit/d66f9dc))
- Fix test_app build ([0b102f6](https://github.com/espressif/esp-protocols/commit/0b102f6))


## [1.0.3](https://github.com/espressif/esp-protocols/commits/4868689)

### Updated

- Updated mDNS to explicitely use esp-eth dependency if needed ([4868689](https://github.com/espressif/esp-protocols/commit/4868689), [IDF@5e19b9c](https://github.com/espressif/esp-idf/commit/5e19b9c9518ae253d82400ab24b86af8af832425))


## [1.0.2](https://github.com/espressif/esp-protocols/commits/8fe2a3a)

### Features

- fix bug when clean action memory ([81c219d](https://github.com/espressif/esp-protocols/commit/81c219d), [IDF@3d4deb9](https://github.com/espressif/esp-idf/commit/3d4deb972620cae8e8ce4d0050153cc6f39db372))

### Bug Fixes

- add the maximum number of services ([0191d6f](https://github.com/espressif/esp-protocols/commit/0191d6f), [IDF@ba458c6](https://github.com/espressif/esp-idf/commit/ba458c69cfb2f18478d73690c289b09641c62004))
- fix the exception when remove one of multiple service ([b26c866](https://github.com/espressif/esp-protocols/commit/b26c866), [IDF@696d733](https://github.com/espressif/esp-idf/commit/696d733eb04ee98f764dffdc82bcef51a724c9c6))

### Updated

- Minor fixes here and there ([8fe2a3a](https://github.com/espressif/esp-protocols/commit/8fe2a3a))
- mDNS: Initial version based on IDF 5.0 ([b6b20ad](https://github.com/espressif/esp-protocols/commit/b6b20ad))
- soc: moved kconfig options out of the target component. ([4a52cf2](https://github.com/espressif/esp-protocols/commit/4a52cf2), [IDF@d287209](https://github.com/espressif/esp-idf/commit/d2872095f93ed82fb91c776081bc1d032493d93e))
- cmake: fix issue with passing cxx_std option for GCC 11, a common workaround ([87c2699](https://github.com/espressif/esp-protocols/commit/87c2699), [IDF@ea0d212](https://github.com/espressif/esp-idf/commit/ea0d2123c806bd0ad77bc49843ee905cf9be65ff))
- kconfig: Changed default values of bool configs - Some bool configs were using default values true and false,   instead of y and n. ([eb536a7](https://github.com/espressif/esp-protocols/commit/eb536a7), [IDF@25c5c21](https://github.com/espressif/esp-idf/commit/25c5c214f38ca690b03533e12fb5a4d774c7eae0))
- esp_netif: Remove tcpip_adapter compatibility layer ([3e93ea9](https://github.com/espressif/esp-protocols/commit/3e93ea9), [IDF@795b7ed](https://github.com/espressif/esp-idf/commit/795b7ed993784e3134195e12b0978504d83dfd56))
- Fix copyright messages, update API descrition ([2c764b1](https://github.com/espressif/esp-protocols/commit/2c764b1), [IDF@42ba8a8](https://github.com/espressif/esp-idf/commit/42ba8a8338fd5efd82498a5989fc5c105938d447))
- Add API to control custom network interfaces ([f836ae7](https://github.com/espressif/esp-protocols/commit/f836ae7), [IDF@b02468d](https://github.com/espressif/esp-idf/commit/b02468dc98d614f931d14cd8b5e2373ca51fb18d))
- CI/mdns: Fix fuzzer build ([4b5f24f](https://github.com/espressif/esp-protocols/commit/4b5f24f), [IDF@98e9426](https://github.com/espressif/esp-idf/commit/98e9426b660a6e825f811cccd45a0722cc801ccd))
- Add support for registering custom netif ([30f37c0](https://github.com/espressif/esp-protocols/commit/30f37c0), [IDF@bec42ff](https://github.com/espressif/esp-idf/commit/bec42ff85d5091d71e1cb1063bea20d7c6ac8c76))
- Indicate interface using esp_netif in search results ([ddc58e8](https://github.com/espressif/esp-protocols/commit/ddc58e8), [IDF@f8495f1](https://github.com/espressif/esp-idf/commit/f8495f1e86de9a8e7d046bf13d0ca04775041b4c))
- Use predefined interfaces to prepare for custom netifs ([fa951bf](https://github.com/espressif/esp-protocols/commit/fa951bf), [IDF@f90b3b7](https://github.com/espressif/esp-idf/commit/f90b3b798b446382d848f8c55c5e1653c81871cd))
- Prepare for dynamic esp-netif support ([605d1fa](https://github.com/espressif/esp-protocols/commit/605d1fa), [IDF@f9892f7](https://github.com/espressif/esp-idf/commit/f9892f77b88ba77dc6608ba746175f6dc64a7607))
- esp_hw_support/esp_system: Re-evaluate header inclusions and include directories ([58bf218](https://github.com/espressif/esp-protocols/commit/58bf218), [IDF@a9fda54](https://github.com/espressif/esp-idf/commit/a9fda54d39d1321005c3bc9b3cc268d0b7e9f052))
- system: move kconfig options out of target component ([ec491ec](https://github.com/espressif/esp-protocols/commit/ec491ec), [IDF@bb88338](https://github.com/espressif/esp-idf/commit/bb88338118957c2214a4c0a33cd4a152e2e1f8ba))
- Update to drop our own packet if bounced back ([94ae672](https://github.com/espressif/esp-protocols/commit/94ae672), [IDF@b5149e3](https://github.com/espressif/esp-idf/commit/b5149e3ee73728f790798e6757d732fe426e21c7))
- Fix potential read behind parsed packet ([e5a3a3d](https://github.com/espressif/esp-protocols/commit/e5a3a3d), [IDF@51a5de2](https://github.com/espressif/esp-idf/commit/51a5de2525d0e82adea2e298a0edcc9b2dee5edd))
- Fix memleak when adding delegated host ([7710ea9](https://github.com/espressif/esp-protocols/commit/7710ea9), [IDF@9cbdb87](https://github.com/espressif/esp-idf/commit/9cbdb8767bdf6e9745e895b2c5af74d0376965e7))
- Fix null-service issue when parsing packets ([034c55e](https://github.com/espressif/esp-protocols/commit/034c55e), [IDF#8307](https://github.com/espressif/esp-idf/issues/8307), [IDF@a57be7b](https://github.com/espressif/esp-idf/commit/a57be7b7d1135ddb29f9da636e9ad315f7fa1fa7))
- Update fuzzer test (add delegation, check memory) ([ec03fec](https://github.com/espressif/esp-protocols/commit/ec03fec), [IDF@2c10071](https://github.com/espressif/esp-idf/commit/2c1007156e01b4707b5c89d73cad05c0eef0264f))
- Remove legacy esp_event API ([5909e9e](https://github.com/espressif/esp-protocols/commit/5909e9e), [IDF@e46aa51](https://github.com/espressif/esp-idf/commit/e46aa515bdf5606a3d868f1034774d5fc96904b8))
- added missing includes ([82e2a5d](https://github.com/espressif/esp-protocols/commit/82e2a5d), [IDF@28d09c7](https://github.com/espressif/esp-idf/commit/28d09c7dbe145ffa6a7dd90531062d4f7669a9c8))
- Clear notification value in mdns_hostname_set ([48e4d40](https://github.com/espressif/esp-protocols/commit/48e4d40), [IDF@83a4ddb](https://github.com/espressif/esp-idf/commit/83a4ddbd250e2b386bccabb4705d4c58c1a22bcb))
- esp_timer: remove legacy ESP32 FRC timer implementation. ([ac6dcb6](https://github.com/espressif/esp-protocols/commit/ac6dcb6), [IDF@edb76f1](https://github.com/espressif/esp-idf/commit/edb76f14d6b3e925568ff04a87befe733ecc4517))
- freertos: Remove legacy data types ([085dbd8](https://github.com/espressif/esp-protocols/commit/085dbd8), [IDF@57fd78f](https://github.com/espressif/esp-idf/commit/57fd78f5baf93a368a82cf4b2e00ca17ffc09115))
- Tools: Custom baud-rate setup is not possible for IDF Monitor from menuconfig anymore ([f78e8cf](https://github.com/espressif/esp-protocols/commit/f78e8cf), [IDF@36a4011](https://github.com/espressif/esp-idf/commit/36a4011ff8985bfbae08ba0272194e6c3ef93bbf))
- Use memcpy() for copy to support non-text TXTs ([6cdf5ee](https://github.com/espressif/esp-protocols/commit/6cdf5ee), [IDF@6aefe9c](https://github.com/espressif/esp-idf/commit/6aefe9c18563ed567d384a956cf02b6f57d6894c))
- Support for null-value TXT records ([fcb5515](https://github.com/espressif/esp-protocols/commit/fcb5515), [IDF#8267](https://github.com/espressif/esp-idf/issues/8267), [IDF@23c2db4](https://github.com/espressif/esp-idf/commit/23c2db406dee8df09dbdba21cb7eef9fbca8bf27))
- Fix alloc issue if TXT has empty value ([9fdbe5f](https://github.com/espressif/esp-protocols/commit/9fdbe5f), [IDF@205f6ba](https://github.com/espressif/esp-idf/commit/205f6ba8541e12d958c7c56af5a7136090f12a0e))
- Fix random crash when defalt service instance queried ([20e6e9e](https://github.com/espressif/esp-protocols/commit/20e6e9e), [IDF@f46dffc](https://github.com/espressif/esp-idf/commit/f46dffca627e9578e49a510580f9754ec1e27e2e))
- Fix minor memory leaks when creating services ([c588263](https://github.com/espressif/esp-protocols/commit/c588263), [IDF@fad62cc](https://github.com/espressif/esp-idf/commit/fad62cc1ed3dce63b58297172a72a489d7af2d9d))
- Fix mDNS memory leak ([6258edf](https://github.com/espressif/esp-protocols/commit/6258edf), [IDF@119b4a9](https://github.com/espressif/esp-idf/commit/119b4a9dd12cf89cc5eaf63f8aa19730607ef30b))
- Fix mDNS memory leak ([c8b0d5e](https://github.com/espressif/esp-protocols/commit/c8b0d5e), [IDF@f5ffd53](https://github.com/espressif/esp-idf/commit/f5ffd53aeb402afc1333a98168bb2fa35d7cdc77))
- Use multi/uni-cast types in API ([5252b1d](https://github.com/espressif/esp-protocols/commit/5252b1d), [IDF@125c312](https://github.com/espressif/esp-idf/commit/125c3125524c71f4f48f635eda12e22fa3bca500))
- Allow for unicast PTR queries ([4e11cc8](https://github.com/espressif/esp-protocols/commit/4e11cc8), [IDF@7eeeb01](https://github.com/espressif/esp-idf/commit/7eeeb01ea705745b027bd8bc11d2b142418e9927))
- Fix potential null deref for ANY query type ([7af91ec](https://github.com/espressif/esp-protocols/commit/7af91ec), [IDF@99dd8ee](https://github.com/espressif/esp-idf/commit/99dd8eedb1a0e957f5f74344e3e4172e61c29ef8))
- Make fuzzer layers compatible with llvm>=6 ([01256d3](https://github.com/espressif/esp-protocols/commit/01256d3), [IDF@1882cbe](https://github.com/espressif/esp-idf/commit/1882cbe44e6140bebb2d27dc18af06dfcb0157f5))
- Fix copyright ([5a2d4ea](https://github.com/espressif/esp-protocols/commit/5a2d4ea), [IDF@c83678f](https://github.com/espressif/esp-idf/commit/c83678f64fe27844fc28050bde6433ccb04a0704))
- Add mDNS miss comment ([9de3f53](https://github.com/espressif/esp-protocols/commit/9de3f53), [IDF@08e0813](https://github.com/espressif/esp-idf/commit/08e081340d9d76d1244e9f2dc527e5ae370b1fbe))
- freertos: remove FREERTOS_ASSERT option ([bcabc8e](https://github.com/espressif/esp-protocols/commit/bcabc8e), [IDF@7255497](https://github.com/espressif/esp-idf/commit/72554971467a5edc9bd6e390cf8fe7b05e6ade81))
- Minor err print fix in socket-networking layer ([dfb27b3](https://github.com/espressif/esp-protocols/commit/dfb27b3), [IDF@f1b8f5c](https://github.com/espressif/esp-idf/commit/f1b8f5c1023df7d649161bc76f2bcc9a8f8f4d8b))
- unified errno format ([076c095](https://github.com/espressif/esp-protocols/commit/076c095), [IDF@87506f4](https://github.com/espressif/esp-idf/commit/87506f46e2922710f48a6b96ca75e53543ff45c4))
- always send A/AAAA records in announcements ([7dd0bc1](https://github.com/espressif/esp-protocols/commit/7dd0bc1), [IDF@456f80b](https://github.com/espressif/esp-idf/commit/456f80b754ebd0bd74e02c7febdf461c6b573b7a))
- filter instance name for ANY queries ([7e82a7c](https://github.com/espressif/esp-protocols/commit/7e82a7c), [IDF@5d0c473](https://github.com/espressif/esp-idf/commit/5d0c47303dd9ead0f2ad291dca1d4b7ce4e23b2b))
- Fix potential null deref reported by fuzzer test ([ae381b7](https://github.com/espressif/esp-protocols/commit/ae381b7), [IDF@cb5653f](https://github.com/espressif/esp-idf/commit/cb5653fd940a9cd41e8554a6d753fab46e0459d7))
- Minor fix of API description and API usage ([941dc5c](https://github.com/espressif/esp-protocols/commit/941dc5c), [IDF@c297301](https://github.com/espressif/esp-idf/commit/c297301ecc350f8315d7eaf78c72b4aba68d422a))
- Added results count to MDNS ([525c649](https://github.com/espressif/esp-protocols/commit/525c649), [IDF@f391d61](https://github.com/espressif/esp-idf/commit/f391d610e8185631b5361dc6c844c4c04aac30b1))
- fix mdns server instance mismatch ([f0839d9](https://github.com/espressif/esp-protocols/commit/f0839d9), [IDF@6173dd7](https://github.com/espressif/esp-idf/commit/6173dd78097216261277c20ebd92a53c68c47f89))
- support multiple instance for mdns service txt set ([69902ea](https://github.com/espressif/esp-protocols/commit/69902ea), [IDF@50f6302](https://github.com/espressif/esp-idf/commit/50f6302c5d7c0498fa1baa6fd6129d8233971a81))
- fix wrong PTR record count ([d0bbe88](https://github.com/espressif/esp-protocols/commit/d0bbe88), [IDF@5d3f815](https://github.com/espressif/esp-idf/commit/5d3f8157e0e481363ef93d54a29d957fc91cca86))
- Build & config: Remove leftover files from the unsupported "make" build system ([4a9d55e](https://github.com/espressif/esp-protocols/commit/4a9d55e), [IDF@766aa57](https://github.com/espressif/esp-idf/commit/766aa5708443099f3f033b739cda0e1de101cca6))
- Build & config: Remove the "make" build system ([be2a924](https://github.com/espressif/esp-protocols/commit/be2a924), [IDF@9c1d4f5](https://github.com/espressif/esp-idf/commit/9c1d4f5b549d6a7125e5c7c323c80d37361991cb))
- freertos: update freertos folder structure to match upstream ([76fcd41](https://github.com/espressif/esp-protocols/commit/76fcd41), [IDF@4846222](https://github.com/espressif/esp-idf/commit/48462221029c7da4b1ea233e9e781cd57ff91c7e))
- support service subtype ([fd8499c](https://github.com/espressif/esp-protocols/commit/fd8499c), [IDF#5508](https://github.com/espressif/esp-idf/issues/5508), [IDF@e7e8610](https://github.com/espressif/esp-idf/commit/e7e8610f563e0b8532a093ea8b803f0eb132fd0e))
- Fix parsing non-standard queries ([38b4fe2](https://github.com/espressif/esp-protocols/commit/38b4fe2), [IDF#7694](https://github.com/espressif/esp-idf/issues/7694), [IDF@d16f9ba](https://github.com/espressif/esp-idf/commit/d16f9bade5beab3785677dd5b39ebc4e9c895008))
- allow mutiple instances with same service type ([b266062](https://github.com/espressif/esp-protocols/commit/b266062), [IDF@b7a99f4](https://github.com/espressif/esp-idf/commit/b7a99f46587a69a2cd07e7616c3bb30b7b1a6edf))
- Update copyright header ([5e087d8](https://github.com/espressif/esp-protocols/commit/5e087d8), [IDF@2a2b95b](https://github.com/espressif/esp-idf/commit/2a2b95b9c22bc5090d87a4f4317288b64b14fcd9))
- Fix potential null dereference identified by fuzzer tests ([91a3d95](https://github.com/espressif/esp-protocols/commit/91a3d95), [IDF@e7dabb1](https://github.com/espressif/esp-idf/commit/e7dabb14f7c8fd9bd2bea55d8f1accc65323a1c0))
- components/bt: move config BT_RESERVE_DRAM from bluedroid to ESP32 controller ([6d6dd2b](https://github.com/espressif/esp-protocols/commit/6d6dd2b), [IDF@b310c06](https://github.com/espressif/esp-idf/commit/b310c062cd25f249e00dd03dd27baed783921630))
- add notification callback for async APIs ([52306e9](https://github.com/espressif/esp-protocols/commit/52306e9), [IDF@986603c](https://github.com/espressif/esp-idf/commit/986603cf07413b46c88c76c324bf500edcfb6171))
- add more mdns result attributes ([d37ab6d](https://github.com/espressif/esp-protocols/commit/d37ab6d), [IDF@76ec76c](https://github.com/espressif/esp-idf/commit/76ec76c12c871554147343bb7141da1e5de58011))
- Add host test using linux target ([5c55ea6](https://github.com/espressif/esp-protocols/commit/5c55ea6), [IDF@fc7e2d9](https://github.com/espressif/esp-idf/commit/fc7e2d9e908f61fb4b852cfae72aa5ff7c662ebc))
- Implement mdns_networking using BSD sockets ([0c71c7b](https://github.com/espressif/esp-protocols/commit/0c71c7b), [IDF@73dfe84](https://github.com/espressif/esp-idf/commit/73dfe84bf295a850edfad39b6b097a71f15964dc))
- fix crash when adding services without hostname set ([4c368c0](https://github.com/espressif/esp-protocols/commit/4c368c0), [IDF@5e98772](https://github.com/espressif/esp-idf/commit/5e98772eaf7e50d96cf2e6ecdfedcd928b61c864))
- Fix fuzzer IDF-mock layer ([af22753](https://github.com/espressif/esp-protocols/commit/af22753), [IDF@619235c](https://github.com/espressif/esp-idf/commit/619235c2ee5a1fe8411bd2be2de8798209f95902))
- Clean the main mdns module from lwip dependencies ([b0957e7](https://github.com/espressif/esp-protocols/commit/b0957e7), [IDF@54e3294](https://github.com/espressif/esp-idf/commit/54e329444a5dd19c51e84b5f1e16455a0f1c6225))
- Add asynchronous query API ([47c7266](https://github.com/espressif/esp-protocols/commit/47c7266), [IDF#7090](https://github.com/espressif/esp-idf/issues/7090), [IDF@d81482d](https://github.com/espressif/esp-idf/commit/d81482d699232b22f4a5cbee2a76199a5285dadb))
- Fix crashes reported by the fuzzer tests ([40da0d2](https://github.com/espressif/esp-protocols/commit/40da0d2), [IDF@4a2e726](https://github.com/espressif/esp-idf/commit/4a2e72677c6fb7681a7e2acd1a878d3deb114079))
- mdns/fuzzer: Fix non-instrumentation test to reproduce fuzzer issues ([5f6b6f9](https://github.com/espressif/esp-protocols/commit/5f6b6f9), [IDF@dae8033](https://github.com/espressif/esp-idf/commit/dae803335e6bc6d9751a360cd3f675ce4027853b))
- return ESP_OK rather than ERR_OK in API functions ([8a12082](https://github.com/espressif/esp-protocols/commit/8a12082), [IDF@2386113](https://github.com/espressif/esp-idf/commit/2386113972ee51ea93e9740d8c34bfe9289ce909))
- fix memory leak in mdns_free when adding delegated hostnames ([46f28a8](https://github.com/espressif/esp-protocols/commit/46f28a8), [IDF@0baee93](https://github.com/espressif/esp-idf/commit/0baee932111268c4a2103e1c1adeb7d99914a937))
- Support for One-Shot mDNS queries ([5a81eae](https://github.com/espressif/esp-protocols/commit/5a81eae), [IDF@f167238](https://github.com/espressif/esp-idf/commit/f167238fac37818aed75dc689eed54ad47528ab9))
- allow explicit txt value length ([2ddaee2](https://github.com/espressif/esp-protocols/commit/2ddaee2), [IDF@b4e0088](https://github.com/espressif/esp-idf/commit/b4e0088b68321acc4698b01faec7e2ffbe1e37c1))
- Fix crashes reported by the fuzzer ([27fc285](https://github.com/espressif/esp-protocols/commit/27fc285), [IDF@79ba738](https://github.com/espressif/esp-idf/commit/79ba738626d643d8c6f32bdcd455e0d2476f94c7))
- Minor correction of the test code ([93e6efe](https://github.com/espressif/esp-protocols/commit/93e6efe), [IDF@7d76245](https://github.com/espressif/esp-idf/commit/7d762451731cb305c3b090509827740f0195a496))
- Fix fuzzer from miss-interpreting adding services as timeouts ([bc4cda8](https://github.com/espressif/esp-protocols/commit/bc4cda8), [IDF@14099fe](https://github.com/espressif/esp-idf/commit/14099fe15efb1b0cde0a8370096c55bba62ff937))
- fix test script delayed response ([8a8d58d](https://github.com/espressif/esp-protocols/commit/8a8d58d), [IDF@a4f2639](https://github.com/espressif/esp-idf/commit/a4f263948c35c13340b6f4b59a649c5073787d5e))
- fix wrong SRV/PTR record handling ([402baeb](https://github.com/espressif/esp-protocols/commit/402baeb), [IDF@e613555](https://github.com/espressif/esp-idf/commit/e6135552d26480e39e11632437020535b1667b7a))
- fix wrong service hostname after mangling ([9fa25ef](https://github.com/espressif/esp-protocols/commit/9fa25ef), [IDF@439b31d](https://github.com/espressif/esp-idf/commit/439b31d065eddfdfb6eb4cf9c00454edfebc3d9b))
- fix empty address change announce packets ([121b525](https://github.com/espressif/esp-protocols/commit/121b525), [IDF@7bbb72d](https://github.com/espressif/esp-idf/commit/7bbb72d86540f04d37b0e2c4efb6dc66ee9c9ea0))
- fix mdns probe/reply behavior ([418fb60](https://github.com/espressif/esp-protocols/commit/418fb60), [IDF@d2a5d25](https://github.com/espressif/esp-idf/commit/d2a5d25984432d149ca31aea4a0d177f3509dd7b))
- make delegate host address a list ([4049b3b](https://github.com/espressif/esp-protocols/commit/4049b3b), [IDF@2d34352](https://github.com/espressif/esp-idf/commit/2d34352f3db0fa71366a838933a29138a90eb2af))
- add remove delegate host api ([c882119](https://github.com/espressif/esp-protocols/commit/c882119), [IDF@2174693](https://github.com/espressif/esp-idf/commit/2174693096b73ce93261611c44ecba647cd01859))
- add mdns delegation ([1eb5df9](https://github.com/espressif/esp-protocols/commit/1eb5df9), [IDF@401ff56](https://github.com/espressif/esp-idf/commit/401ff56cc1ad1d11284143a348cc0c0e4a363e98))
- fix memory free issue when repeating the query in reply ([b62b4b3](https://github.com/espressif/esp-protocols/commit/b62b4b3), [IDF@5f244c8](https://github.com/espressif/esp-idf/commit/5f244c86f29da46c17610563a245d1663a46b439))
- Fix of crash when wifi interface get deleted and mdns receives the packets ([4d8aec1](https://github.com/espressif/esp-protocols/commit/4d8aec1), [IDF#6973](https://github.com/espressif/esp-idf/issues/6973), [IDF@03de74a](https://github.com/espressif/esp-idf/commit/03de74a728d4b278f55e1fc30e0425483b806e80))
- Docs: Added README.md for lwip fuzzer tests ([6d64910](https://github.com/espressif/esp-protocols/commit/6d64910), [IDF@53c18a8](https://github.com/espressif/esp-idf/commit/53c18a85db104bb37ebeadec2faf5d42d764d0f9))
- Fixed the ip header TTL to be correctly set to 255 ([ab3fa69](https://github.com/espressif/esp-protocols/commit/ab3fa69), [IDF@5cce919](https://github.com/espressif/esp-idf/commit/5cce919cbef87f543bb9f5275b77b97b3b1ea67e))
- Fix parsing answers with questions when instance name not set ([c3a5826](https://github.com/espressif/esp-protocols/commit/c3a5826), [IDF#6598](https://github.com/espressif/esp-idf/issues/6598), [IDF@3404945](https://github.com/espressif/esp-idf/commit/34049454dfaf5132d9b258ef4d04921befc8997b))
- Fix the resolver to correctly parse it's own non-strict answers ([cbcbe4f](https://github.com/espressif/esp-protocols/commit/cbcbe4f), [IDF@b649603](https://github.com/espressif/esp-idf/commit/b649603a0d70ec804567f57752c3eddaed56198f))
- Add MDNS_STRICT_MODE config option ([adc3430](https://github.com/espressif/esp-protocols/commit/adc3430), [IDF@0eee315](https://github.com/espressif/esp-idf/commit/0eee31546dd4e6df0d1c1cc2740da0675dffb4bf))
- freertos: common config header ([c30617d](https://github.com/espressif/esp-protocols/commit/c30617d), [IDF@39cf818](https://github.com/espressif/esp-idf/commit/39cf818838b0259b3e00b3c198ad47b4add41939))
- Removed freeRTOS dependancies from fuzzer tests ([1e5eeb1](https://github.com/espressif/esp-protocols/commit/1e5eeb1), [IDF@5571694](https://github.com/espressif/esp-idf/commit/55716945a9908e057743d69e1d59399df03e49bd))
- mDNS: Updated APIs description and shows the warning when hostname contains domain name during the query ([22c7c0a](https://github.com/espressif/esp-protocols/commit/22c7c0a), [IDF#6590](https://github.com/espressif/esp-idf/issues/6590), [IDF@9f8d2b9](https://github.com/espressif/esp-idf/commit/9f8d2b944d2b3736a012e0dff1a8459b6941d295))
- components: Use CONFIG_LWIP_IPV6 to strip IPv6 function in components ([1623c0e](https://github.com/espressif/esp-protocols/commit/1623c0e), [IDF@da58235](https://github.com/espressif/esp-idf/commit/da58235a0ee262ff552c5f1155d531b5c31e8de6))
- add bound check when setting interface as duplicate ([b114ed6](https://github.com/espressif/esp-protocols/commit/b114ed6), [IDF@2b9d2c0](https://github.com/espressif/esp-idf/commit/2b9d2c06f54924b680c41ae641978c8d81612f65))
- mDNS: Fix of text length calculation when detecting a collision ([2ffd223](https://github.com/espressif/esp-protocols/commit/2ffd223), [IDF@be0ae1e](https://github.com/espressif/esp-idf/commit/be0ae1ebbbe9fae6ecf7de09e8d50cba063b61f4))
- global: fix sign-compare warnings ([1fe901f](https://github.com/espressif/esp-protocols/commit/1fe901f), [IDF@753a929](https://github.com/espressif/esp-idf/commit/753a9295259126217a9fe6ef1c5e9da21e9b4e28))
- lwip: Moved default SNTP API to esp_sntp.h ([2cf9fd8](https://github.com/espressif/esp-protocols/commit/2cf9fd8), [IDF@76f6dd6](https://github.com/espressif/esp-idf/commit/76f6dd6214ca583b1a94c7c553ccac739a27f6d5))
- Allow resolve its own non-strict answers ([89439e0](https://github.com/espressif/esp-protocols/commit/89439e0), [IDF#6190](https://github.com/espressif/esp-idf/issues/6190), [IDF@0693e17](https://github.com/espressif/esp-idf/commit/0693e172de392086b9bfd8cf1474d8d133af3298))
- mDNS: Fix of collision detection during txt length calculation ([becd5d0](https://github.com/espressif/esp-protocols/commit/becd5d0), [IDF#6114](https://github.com/espressif/esp-idf/issues/6114), [IDF@f33772c](https://github.com/espressif/esp-idf/commit/f33772c96037c795366e60082bdbbefe2a69165f))
- esp32c3: Apply one-liner/small changes for ESP32-C3 ([0d7a309](https://github.com/espressif/esp-protocols/commit/0d7a309), [IDF@5228d9f](https://github.com/espressif/esp-idf/commit/5228d9f9ced16118d87326f94d9f9dfd411e0be9))
- test: fix several test build error ([1fdffbb](https://github.com/espressif/esp-protocols/commit/1fdffbb), [IDF@b7ecccd](https://github.com/espressif/esp-idf/commit/b7ecccd9010f1deaba83de54374231c3c7f5b472))
- freertos: Add RISC-V port ([988d120](https://github.com/espressif/esp-protocols/commit/988d120), [IDF@87e13ba](https://github.com/espressif/esp-idf/commit/87e13baaf12fe6deae715d95e912a310fea4ba88))
- Fix wrong mdns source address if lwIP IPv6 zones disabled ([fd47df3](https://github.com/espressif/esp-protocols/commit/fd47df3), [IDF@7ac9761](https://github.com/espressif/esp-idf/commit/7ac97616c119e4d2f4cdd377dfc5abbf75ec5e30))
- Whitespace: Automated whitespace fixes (large commit) ([2cb3a6e](https://github.com/espressif/esp-protocols/commit/2cb3a6e), [IDF@66fb5a2](https://github.com/espressif/esp-idf/commit/66fb5a29bbdc2482d67c52e6f66b303378c9b789))
- test_compile_fuzzers: Fix include paths for host build ([825652f](https://github.com/espressif/esp-protocols/commit/825652f), [IDF@98a0cc7](https://github.com/espressif/esp-idf/commit/98a0cc783f701b238bea232b53250a538d34920a))
- CI: Add a test to pre-check fuzzer tests compilation before weekly run ([fc53888](https://github.com/espressif/esp-protocols/commit/fc53888), [IDF@637f5c0](https://github.com/espressif/esp-idf/commit/637f5c0a6842c42ee6cf7f41d3c5ae0cb28a68af))
- soc: descriptive part occupy whole component ([7635c04](https://github.com/espressif/esp-protocols/commit/7635c04), [IDF@79887fd](https://github.com/espressif/esp-idf/commit/79887fdc6c3d9a2e509cc189bb43c998d3f0f4ee))
- Coredump config option rename throughout IDF ([d5fe42b](https://github.com/espressif/esp-protocols/commit/d5fe42b), [IDF@20af94f](https://github.com/espressif/esp-idf/commit/20af94ff53c5147a76342800d007a6c49be50a7b))
- mdns, dns, dhcp, dhcps: update fuzzer test to work in CI ([e0bc60a](https://github.com/espressif/esp-protocols/commit/e0bc60a), [IDF@a43c06a](https://github.com/espressif/esp-idf/commit/a43c06a592bcf9404297b22268c33bb7a246632c))
- cmock: added cmock as component ([9772e49](https://github.com/espressif/esp-protocols/commit/9772e49), [IDF@20c068e](https://github.com/espressif/esp-idf/commit/20c068ef3b49999387896b90f8011b02f718485f))
- Support queries in responses in mDNS non-strict mode ([6021a88](https://github.com/espressif/esp-protocols/commit/6021a88), [IDF#5521](https://github.com/espressif/esp-idf/issues/5521), [IDF@bcfa36d](https://github.com/espressif/esp-idf/commit/bcfa36db8ffff997f1f95eaf6b011ffc4d46a10f))
- Fix include query ID in reponses ([78f71ec](https://github.com/espressif/esp-protocols/commit/78f71ec), [IDF#5574](https://github.com/espressif/esp-idf/issues/5574), [IDF@f62e321](https://github.com/espressif/esp-idf/commit/f62e321d87c1d520cccca951715c27730e06607a))
- Allow config mDNS task stack size ([3319844](https://github.com/espressif/esp-protocols/commit/3319844), [IDF@cf7e48c](https://github.com/espressif/esp-idf/commit/cf7e48c779edd84c3f99d5e8ed81027932302382))
- Remove mbedtls dependency ([ac70c9a](https://github.com/espressif/esp-protocols/commit/ac70c9a), [IDF@f4a4549](https://github.com/espressif/esp-idf/commit/f4a4549a344e7ff2444a188adbebbc136b47a7bb))
- limit the GOT_IP6_EVENT to only known network interfaces ([2b7d43e](https://github.com/espressif/esp-protocols/commit/2b7d43e), [IDF@ab8cab1](https://github.com/espressif/esp-idf/commit/ab8cab1c553ee5312ef47a7dea002f2585605006))
- esp32: add implementation of esp_timer based on TG0 LAC timer ([4eb3e89](https://github.com/espressif/esp-protocols/commit/4eb3e89), [IDF@739eb05](https://github.com/espressif/esp-idf/commit/739eb05bb97736b70507e7ebcfee58e670672d23))
- fixed typos in the variable names and the comments ([b5e5a64](https://github.com/espressif/esp-protocols/commit/b5e5a64), [IDF@ecca39e](https://github.com/espressif/esp-idf/commit/ecca39e19f663e32e16aef2a09df15443de347e9))
- fix preset of esp_netif ptr for local interfaces ([6713ffe](https://github.com/espressif/esp-protocols/commit/6713ffe), [IDF@09e36f9](https://github.com/espressif/esp-idf/commit/09e36f9f3354092b2a528baaaaccab28ff4774d6))
- fixed crash on event during deinit ([817c4fd](https://github.com/espressif/esp-protocols/commit/817c4fd), [IDF@eaa2f12](https://github.com/espressif/esp-idf/commit/eaa2f12d6761710d2633b4934fe09f6f45e20f4f))
- respond to discovery with the proper pseudo name _services._dns-sd._udp ([8f0dc6d](https://github.com/espressif/esp-protocols/commit/8f0dc6d), [IDF#4369](https://github.com/espressif/esp-idf/issues/4369), [IDF@de17a14](https://github.com/espressif/esp-idf/commit/de17a1487f8ba6f432b06199f2261132ec6e735f))
- fixed forgotten merge conflicts in debug code ([d20666f](https://github.com/espressif/esp-protocols/commit/d20666f), [IDF@d9433ef](https://github.com/espressif/esp-idf/commit/d9433ef69223a32d05abdca543fb530f2e6679e4))
- add missing include of esp_task.h ([662a4ce](https://github.com/espressif/esp-protocols/commit/662a4ce), [IDF@5884b80](https://github.com/espressif/esp-idf/commit/5884b80908d680874e27fa0c8b2df85b69d03dd3))
- add configuration values for task priority, affinity and internal service timeouts ([fb1de80](https://github.com/espressif/esp-protocols/commit/fb1de80), [IDF@c6f38f0](https://github.com/espressif/esp-idf/commit/c6f38f04f8eec1aae937cc87c111609772681cb3))
- tcpip_adapter: updated tcpip_adapter compatablity layer to include all public API and keep 100% backward compatibility update build of tcpip adapter when ethernet disabled ([1f35e9a](https://github.com/espressif/esp-protocols/commit/1f35e9a), [IDF@7f5cda1](https://github.com/espressif/esp-idf/commit/7f5cda1b825586903f85dc4ad7736b35712e46d7))
- update mdns to use esp-netif for mdns supported services such as STA, AP, ETH ([48b819b](https://github.com/espressif/esp-protocols/commit/48b819b), [IDF@19e24fe](https://github.com/espressif/esp-idf/commit/19e24fe61ed5ea6698dfd5e1f427e783360aa846))
- esp_netif: Introduction of esp-netif component as a replacement of tcpip_adpter ([53e2aa3](https://github.com/espressif/esp-protocols/commit/53e2aa3), [IDF@ffe043b](https://github.com/espressif/esp-idf/commit/ffe043b1a81a0f9e1cc2cfa8873e21318ec89143))
- examples: removed ip4addr_ntoa and used prefered IP2STR for displaying IP addresses ([3cc6446](https://github.com/espressif/esp-protocols/commit/3cc6446), [IDF@ec9f245](https://github.com/espressif/esp-idf/commit/ec9f245dd35d3e8e7b19a8dec5e05e003dc21f39))
- esp_event, mdns: fixes for CONFIG_ETH_ENABLED=n ([248b11b](https://github.com/espressif/esp-protocols/commit/248b11b), [IDF@569ad75](https://github.com/espressif/esp-idf/commit/569ad7545c32a2f1d0eff3f1e81df70fb76ad125))
- build and link hello-world for esp32s2beta ([901124b](https://github.com/espressif/esp-protocols/commit/901124b), [IDF@84b2f9f](https://github.com/espressif/esp-idf/commit/84b2f9f14d16533c84db2210f13a24cd817e0b0a))
- fix crash for hostname queries ([f6ff165](https://github.com/espressif/esp-protocols/commit/f6ff165), [IDF#4224](https://github.com/espressif/esp-idf/issues/4224), [IDF@3d11700](https://github.com/espressif/esp-idf/commit/3d1170031b340a231949fdc0d9c46d87af0d1b5d))
- fix possible race condition when checking DHCP status on WIFI_EVENT_STA_CONNECTED event. ([f44c569](https://github.com/espressif/esp-protocols/commit/f44c569), [IDF@7f410a0](https://github.com/espressif/esp-idf/commit/7f410a0bcbafa85dba05807c53c3c38999506509))
- use constant size of AAAA answer in mdns packets instead of deriving from lwip struct size, since the struct could contain also zones ([286c646](https://github.com/espressif/esp-protocols/commit/286c646), [IDF@e5e31c5](https://github.com/espressif/esp-idf/commit/e5e31c5d0172d68fd207fa31cc5d3bba82dad020))
- esp_wifi: wifi support new event mechanism ([c70d527](https://github.com/espressif/esp-protocols/commit/c70d527), [IDF@003a987](https://github.com/espressif/esp-idf/commit/003a9872b7de69d799e9d37521cfbcaff9b37e85))
- fix missing bye packet if services removed with mdns_service_remove_all() or mdns_free() ([7cdf96c](https://github.com/espressif/esp-protocols/commit/7cdf96c), [IDF#3660](https://github.com/espressif/esp-idf/issues/3660), [IDF@a001998](https://github.com/espressif/esp-idf/commit/a001998d5283b29ca9a374adf7cef3357b39a03a))
- mdns_service_remove_all doesn't take an argument ([407875d](https://github.com/espressif/esp-protocols/commit/407875d), [IDF@c2764f6](https://github.com/espressif/esp-idf/commit/c2764f6fe85681cfaf5dbbe168295284f09c09cd))
- tools: Mass fixing of empty prototypes (for -Wstrict-prototypes) ([3e753f5](https://github.com/espressif/esp-protocols/commit/3e753f5), [IDF@afbaf74](https://github.com/espressif/esp-idf/commit/afbaf74007e89d016dbade4072bf2e7a3874139a))
- fix ignoring mdns packet with some invalid name entries in question field ([144d4ad](https://github.com/espressif/esp-protocols/commit/144d4ad), [IDF@4bd4c7c](https://github.com/espressif/esp-idf/commit/4bd4c7caf3f9ef8402c5a27ab44561537407eb60))
- add esp_eth component ([680bad6](https://github.com/espressif/esp-protocols/commit/680bad6), [IDF@90c4827](https://github.com/espressif/esp-idf/commit/90c4827bd22aa61894a5b22b3b39247a7e44d6cf))
- components: use new component registration api ([7fb6686](https://github.com/espressif/esp-protocols/commit/7fb6686), [IDF@9eccd7c](https://github.com/espressif/esp-idf/commit/9eccd7c0826d6cc2e9de59304d1e5f76c0063ccf))
- fix static analysis warnings ([4912bef](https://github.com/espressif/esp-protocols/commit/4912bef), [IDF@c34de4c](https://github.com/espressif/esp-idf/commit/c34de4cba658e8331f8a3ab2f466190c7640595b))
- added initial suite of api unit tests ([181a22e](https://github.com/espressif/esp-protocols/commit/181a22e), [IDF@e680191](https://github.com/espressif/esp-idf/commit/e6801912c5c4861f828ab1f447280628bba9a5d7))
- mdns tests: adapt mdns fuzzer test to compile with event loop library ([4172219](https://github.com/espressif/esp-protocols/commit/4172219), [IDF@38d15cb](https://github.com/espressif/esp-idf/commit/38d15cbd637e8b8baacda9fc43e8e99d224530f5))
- fixed mdns crashing on reception of txt packet without a corresponding service closes #2866 ([98d2c1a](https://github.com/espressif/esp-protocols/commit/98d2c1a), [IDF@af48977](https://github.com/espressif/esp-idf/commit/af48977f21cea6b18dae10b2c8b64a78acfc647f))
- use const char* for mdns txt items types to remove warning when assigning ([84cbb1f](https://github.com/espressif/esp-protocols/commit/84cbb1f), [IDF@c050a75](https://github.com/espressif/esp-idf/commit/c050a75616803c7871ef11c060e440fae09000d9))
- updated doxygen comments documenting mdns api ([4c6818e](https://github.com/espressif/esp-protocols/commit/4c6818e), [IDF#1718](https://github.com/espressif/esp-idf/issues/1718), [IDF@a851aac](https://github.com/espressif/esp-idf/commit/a851aac255311124529f504486ca55bad15c1951))
- update mdns_out_question_s to be in line with mdns_parsed_question_s struct ([c440114](https://github.com/espressif/esp-protocols/commit/c440114), [IDF#1568]( https://github.com/espressif/esp-idf/issues/1568), [IDF@eddd5c4](https://github.com/espressif/esp-idf/commit/eddd5c4f2c686d9a1d6d3258569cc33752e78880))
- use esp_event library to handle events ([6ea0ea9](https://github.com/espressif/esp-protocols/commit/6ea0ea9), [IDF@a2d5952](https://github.com/espressif/esp-idf/commit/a2d59525e53099ee1ad63c3d60ff853f573ab535))
- fuzzer tests: update of mdns and lwip host compilation for fuzzer testing ([d9aec9f](https://github.com/espressif/esp-protocols/commit/d9aec9f), [IDF@bc60bbb](https://github.com/espressif/esp-idf/commit/bc60bbbeaf89f2bbfc5db4bd4f1e7ace81a2ab37))
- fix possible crash when probing on particular interface with duplicated service instances due to naming conflicts on network ([985e691](https://github.com/espressif/esp-protocols/commit/985e691), [IDF@265e983](https://github.com/espressif/esp-idf/commit/265e983a452a7eaefc1662cdc0e6ed839a37fe1a))
- enable pcbs before starting service thread to avoid updating pcb's internal variables from concurent tasks ([75deebb](https://github.com/espressif/esp-protocols/commit/75deebb), [IDF@c87f0cb](https://github.com/espressif/esp-idf/commit/c87f0cb6cad3c36b077f4aaeb1ca52fe6ed0cdaf))
- fix possible deadlock on mdns deinit calling mdns_free() ([fdd27dc](https://github.com/espressif/esp-protocols/commit/fdd27dc), [IDF#1696](https://github.com/espressif/esp-idf/issues/1696), [IDF@48b5501](https://github.com/espressif/esp-idf/commit/48b5501c250ed90da51a55ad4fc18e09f466a517))
- mdsn: fix race condition in updating packet data from user task when failed to allocate or queue a new service ([2ec3b55](https://github.com/espressif/esp-protocols/commit/2ec3b55), [IDF@021dc5d](https://github.com/espressif/esp-idf/commit/021dc5d453e21e2d1707f194668e69cf63ef4e84))
- fix possible crash when packet scheduled to transmit contained service which might have been already removed ([450cbf0](https://github.com/espressif/esp-protocols/commit/450cbf0), [IDF@67051a2](https://github.com/espressif/esp-idf/commit/67051a286ba60a01d4755c3682129153c2f95953))
- use binary semaphore instead of mutex when searching ([34f6d8d](https://github.com/espressif/esp-protocols/commit/34f6d8d), [IDF@eef0b50](https://github.com/espressif/esp-idf/commit/eef0b5090aee87efef1a6a37772b3b88c9ce8df8))
- fix memory leak in pbuf if tcpipadapter failed to get netif ([b6efc68](https://github.com/espressif/esp-protocols/commit/b6efc68), [IDF@8462751](https://github.com/espressif/esp-idf/commit/8462751f95a3ff18bdc1b01d02fabd1829fd9135))
- fix malfuctional query_txt ([90e4bab](https://github.com/espressif/esp-protocols/commit/90e4bab), [IDF@1a02773](https://github.com/espressif/esp-idf/commit/1a027734af06abf08fcb1c34ee65bdf50d12be4d))
- fix possible crash when mdns_free called while action queue not empty ([c546ab8](https://github.com/espressif/esp-protocols/commit/c546ab8), [IDF@206b47c](https://github.com/espressif/esp-idf/commit/206b47c03aca0acdf40d1d9c250e18aeddfe1bd7))
- fix memory leak when query for service receives multiple ptr entries for one instance ([6582b41](https://github.com/espressif/esp-protocols/commit/6582b41), [IDF@9a4da97](https://github.com/espressif/esp-idf/commit/9a4da97fb4b3c241998cb969a08c3a917ffb4cd1))
- fix crash after init if no memory for task ([358d26c](https://github.com/espressif/esp-protocols/commit/358d26c), [IDF@a47768d](https://github.com/espressif/esp-idf/commit/a47768dc4e4750fd7e1c29b15d6e2dd3c76e6591))
- fixed crash on free undefined ptr after skipped strdup ([2ac83d0](https://github.com/espressif/esp-protocols/commit/2ac83d0), [IDF@e0a8044](https://github.com/espressif/esp-idf/commit/e0a8044a16907e642001b883469618a999dbe6db))
- Correct Kconfigs according to the coding style ([98e3171](https://github.com/espressif/esp-protocols/commit/98e3171), [IDF@37126d3](https://github.com/espressif/esp-idf/commit/37126d3451eabb44eeeb48b8e2ee554dc233e2a8))
- fix networking running udp_sendif from lwip thread ([2f85c07](https://github.com/espressif/esp-protocols/commit/2f85c07), [IDF@f7d4a4b](https://github.com/espressif/esp-idf/commit/f7d4a4be6a9e0b0ac5edb9400d3b123dbbed2ffc))
- fixed static memory leak ([b30a7fe](https://github.com/espressif/esp-protocols/commit/b30a7fe), [IDF@6bb68a5](https://github.com/espressif/esp-idf/commit/6bb68a5a7567a94c3605136d44960ff060c74663))
- check all mallocs for failure and add default hook to log error with free heap ([7a4fdad](https://github.com/espressif/esp-protocols/commit/7a4fdad), [IDF@c8cb4cd](https://github.com/espressif/esp-idf/commit/c8cb4cd3c8eb56d5901ade03302ad1231d7f3de5))
- resolve memory leak when txt record received multiple times ([b4e5742](https://github.com/espressif/esp-protocols/commit/b4e5742), [IDF@a6b2b73](https://github.com/espressif/esp-idf/commit/a6b2b73f03bbb75a39685ddba6cf877fd1e5e6d7))
- skip sending search when finished, not properly locked timer task ([2763bcd](https://github.com/espressif/esp-protocols/commit/2763bcd), [IDF@31163f0](https://github.com/espressif/esp-idf/commit/31163f02d5c414d8b492dce6f729b43a0061581b))
- sending search packets also in probing and announcing state ([8cd0e8a](https://github.com/espressif/esp-protocols/commit/8cd0e8a), [IDF@d16762a](https://github.com/espressif/esp-idf/commit/d16762a036e35ce86ece86bb44e6e99f9cc7c431))
- fixed crashes on network changes ([9b3b41c](https://github.com/espressif/esp-protocols/commit/9b3b41c), [IDF@097282a](https://github.com/espressif/esp-idf/commit/097282a8e3f85958747430d9931ce0a545d37700))
- Update network code for mDNS to work with newest LwIP ([ea23007](https://github.com/espressif/esp-protocols/commit/ea23007), [IDF@3ec0e7e](https://github.com/espressif/esp-idf/commit/3ec0e7e2d2ddea70e9f8fb5025664d0fe24c301a))
- bugfix: mdns_service_txt_set() wasn't allocating memory for TXT records ([0c17121](https://github.com/espressif/esp-protocols/commit/0c17121), [IDF@e5e2702](https://github.com/espressif/esp-idf/commit/e5e2702ca3f63a29da57eb138f75a20c74fb2a94))
- cmake: make main a component again ([67173f6](https://github.com/espressif/esp-protocols/commit/67173f6), [IDF@d9939ce](https://github.com/espressif/esp-idf/commit/d9939cedd9b44d63dc148354c3a0a139b9c7113d))
- Feature/sync lwip as submodule ([fed787f](https://github.com/espressif/esp-protocols/commit/fed787f), [IDF@3578fe3](https://github.com/espressif/esp-idf/commit/3578fe39e01ba0c2d54824ac70c3276502661c6b))
- Fix a portion of the queries are issued with the wildcard query type ([b4ab30b](https://github.com/espressif/esp-protocols/commit/b4ab30b), [IDF@f3f0445](https://github.com/espressif/esp-idf/commit/f3f0445f4db7c9ad97ae10a9728767337aa7bb62))
- added CI job for AFL fuzzer tests ([dd71494](https://github.com/espressif/esp-protocols/commit/dd71494), [IDF@0c14764](https://github.com/espressif/esp-idf/commit/0c147648f7642d058b63fbe2ddd5de31c2326304))
- Minor fix for mdns_service_remove() ([39de491](https://github.com/espressif/esp-protocols/commit/39de491), [IDF@5c7eb7e](https://github.com/espressif/esp-idf/commit/5c7eb7e27be7508130459d896cf7d13ffefda87f))
- Replace all DOS line endings with Unix ([19acac7](https://github.com/espressif/esp-protocols/commit/19acac7), [IDF@a67d5d8](https://github.com/espressif/esp-idf/commit/a67d5d89e0e90390fa7ff02816a6a79008d75d40))
- remove executable permission from source files ([98069f9](https://github.com/espressif/esp-protocols/commit/98069f9), [IDF@cb649e4](https://github.com/espressif/esp-idf/commit/cb649e452f3c64a7db1f4a61e162a16b70248204))
- Fixed nullptr dereference in MDNS.c ([ad29d34](https://github.com/espressif/esp-protocols/commit/ad29d34), [IDF@fffbf7b](https://github.com/espressif/esp-idf/commit/fffbf7b75065b5852e064e04b0c5102dd0fc2244))
- MDNS-Fuzzer: AFL fuzzer tests for mdsn packet parser ([9f1be36](https://github.com/espressif/esp-protocols/commit/9f1be36), [IDF@e983230](https://github.com/espressif/esp-idf/commit/e983230be933fb83cebdd1945ba6539a7dc99b28))
- cmake: Add component dependency support ([c7701d4](https://github.com/espressif/esp-protocols/commit/c7701d4), [IDF@1cb5712](https://github.com/espressif/esp-idf/commit/1cb5712463a8963cd3e8331da90fb5e03f13575f))
- cmake: Remove defaults for COMPONENT_SRCDIRS, COMPONENT_SRCS, COMPONENT_ADD_INCLUDEDIRS ([f1ccc40](https://github.com/espressif/esp-protocols/commit/f1ccc40), [IDF@4f1a856](https://github.com/espressif/esp-idf/commit/4f1a856dbfd752336cd71730105e02ad8c045541))
- build system: Initial cmake support, work in progress ([84bd1d7](https://github.com/espressif/esp-protocols/commit/84bd1d7), [IDF@c671a0c](https://github.com/espressif/esp-idf/commit/c671a0c3ebf90f18576d6db55b51b62730c58301))
- fix the bug that in mdns test code redefine esp_err_t to uint32_t, which should be int32_t ([259d3fc](https://github.com/espressif/esp-protocols/commit/259d3fc), [IDF@81e4cad](https://github.com/espressif/esp-idf/commit/81e4cad61593cde879a5c44a08060d9d336e5a3f))
- Fix exception when service is removed while there are pending packets that depend on it ([7784d00](https://github.com/espressif/esp-protocols/commit/7784d00), [IDF@421c6f1](https://github.com/espressif/esp-idf/commit/421c6f154b10d9253b78875ab28ee6bdcaaaf3c0))
- Fix case where service is NULL and that will cause exception ([bce7d52](https://github.com/espressif/esp-protocols/commit/bce7d52), [IDF@4fa130a](https://github.com/espressif/esp-idf/commit/4fa130ae4fb5de99ddddc5a7bed7e26ae645591c))
- Fix issue with some mDNS parsers ([ef924f1](https://github.com/espressif/esp-protocols/commit/ef924f1), [IDF@51dde19](https://github.com/espressif/esp-idf/commit/51dde19a765533af67fc7be21f116641773a9be4))
- Import mDNS changes ([ad8c92d](https://github.com/espressif/esp-protocols/commit/ad8c92d), [IDF@4bddbc0](https://github.com/espressif/esp-idf/commit/4bddbc031cee83935c0e4df1dc72cc7000c97ba5))
- Fix compilation errors when using gcc-7.2.0 for the crosstool-ng toolchain ([3aa605f](https://github.com/espressif/esp-protocols/commit/3aa605f), [IDF@519edc3](https://github.com/espressif/esp-idf/commit/519edc332dae0160069fd790467cde8de78f1a0e))
- components/mdns: wrong Message compression detect ([00a72b8](https://github.com/espressif/esp-protocols/commit/00a72b8), [IDF@6e24566](https://github.com/espressif/esp-idf/commit/6e24566186c52dc5432b6b25c81abda577c21e85))
- fix leak after _mdns_create_service if we have a malloc error. ([907e7ee](https://github.com/espressif/esp-protocols/commit/907e7ee), [IDF@b6b36bd](https://github.com/espressif/esp-idf/commit/b6b36bd9ddf169039a5528f8b766048d97b975f7))
- Use LwIP IPC for low-level API calls ([b367484](https://github.com/espressif/esp-protocols/commit/b367484), [IDF@713964f](https://github.com/espressif/esp-idf/commit/713964fe9e98b4fa34145c497b7ab638dc57614c))
- Add AFL fuzz test ([4a8582f](https://github.com/espressif/esp-protocols/commit/4a8582f), [IDF@4c26227](https://github.com/espressif/esp-idf/commit/4c2622755d92efa1818d062d433725553437993c))
- implement fixes for issues found while fuzz testing ([75de31c](https://github.com/espressif/esp-protocols/commit/75de31c), [IDF@99d3990](https://github.com/espressif/esp-idf/commit/99d39909c4f19c63909d663e927ac0a8933a3ed5))
- add simple dns-sd meta query support ([4acf639](https://github.com/espressif/esp-protocols/commit/4acf639), [IDF@96e8a3c](https://github.com/espressif/esp-idf/commit/96e8a3c725095562d2725aaefa15adcfc5d78dd5))
- address security issues with mDNS ([91bb509](https://github.com/espressif/esp-protocols/commit/91bb509), [IDF@c89e11c](https://github.com/espressif/esp-idf/commit/c89e11c8fa64641edddf9a055745d825ae3fab9d))
- Initial mDNS component and example ([7fbf8e5](https://github.com/espressif/esp-protocols/commit/7fbf8e5), [IDF@dd3f18d](https://github.com/espressif/esp-idf/commit/dd3f18d2d88ee78909d4af2840dfdf0b9f715f28))
