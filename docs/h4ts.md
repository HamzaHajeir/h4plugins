![H4P Flyer](../assets/H4PLogo.png)

# H4P_TaskSniffer

## Service shortname snif

Provides task execution monitoring and debugging by hooking into the H4 task scheduler.

**Note:** This plugin is only available if `H4_HOOK_TASKS` is defined during compilation.

---

## Contents

* [Usage](#usage)
* [Dependencies](#dependencies)
* [Commands Added](#commands-added)
* [Service Commands](#service-commands)
* [API](#api)

---

# Usage

```cpp
H4P_TaskSniffer snif; // Monitor all tasks
// or
H4P_TaskSniffer snif(5); // Monitor task ID 5
// or
H4P_TaskSniffer snif({1,2,3}); // Monitor tasks 1,2,3
```

This plugin is a "singleton" - there may be only one single instance of it in the app. 
It may be instantiated as any name the user chooses, prefix all API calls below with that name.

# Dependencies

N/A

# Commands Added

| Command | Parameters | Purpose |
|---------|------------|---------|
| include | task_ids | Add task IDs to monitor (comma-separated, or range with -) |
| exclude | task_ids | Remove task IDs from monitoring (comma-separated, or range with -) |

# Service Commands

`stop` will disconnect from TaskSniffer and initiate closedown of all Plugins the depend on TaskSniffer
`start` will connect to TaskSniffer and start task monitoring

---

## API

```cpp
/*
Constructors
*/
H4P_TaskSniffer(); // Monitor all tasks (0-99)
H4P_TaskSniffer(uint32_t i); // Monitor specific task ID
H4P_TaskSniffer(std::initializer_list<uint32_t> i); // Monitor list of task IDs

/*
Monitoring control
*/
void include(uint32_t i); // Add single task ID to monitor
void include(std::initializer_list<uint32_t> i); // Add list of task IDs
void include(std::vector<uint32_t> i); // Add vector of task IDs

void exclude(uint32_t i); // Remove single task ID from monitoring
void exclude(std::initializer_list<uint32_t> i); // Remove list of task IDs
void exclude(std::vector<uint32_t> i); // Remove vector of task IDs
```

# Example sketches

See examples in the [examples/](../examples/) folder for task debugging usage.

---

(c) 2021 Phil Bowles
(c) 2025 Hamza Hajeir

* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9BUtleZRY6g)
* [Facebook H4  Support / Discussion](https://www.facebook.com/groups/444344099599131/)
* [Facebook General ESP8266 / ESP32](https://www.facebook.com/groups/2125820374390340/)
* [Facebook ESP8266 Programming Questions](https://www.facebook.com/groups/esp8266questions/)
* [Facebook ESP Developers (moderator)](https://www.facebook.com/groups/ESP8266/)
* [Support me on Patreon](https://patreon.com/esparto)