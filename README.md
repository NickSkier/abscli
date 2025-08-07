# abscli

`abscli` is a command-line interface for the [audiobookshelf](https://github.com/advplyr/audiobookshelf) server, designed to provide a simple and efficient way to manage your server directly from the terminal. This project is currently under development.

## Current Status

**Work in Progress:** This project is not yet finished. Right now it is not intendent for actual usage.

## Features

- **User Authentication:**
  - Log in to your audiobookshelf server.
  - Securely stores refresh tokens in the system's keychain.
  - Fetches new access tokens automatically.
- **Data Synchronization:**
  - Sync user profile data.
  - Sync libraries.
  - Sync library items.
- **Local Caching:**
  - Uses an SQLite database to cache server data locally for faster access and offline browsing.
- **Interactive CLI:**
  - An interactive menu to navigate through different functionalities.
  - List synced libraries, library items, and books.

## Build

```bash
git clone https://github.com/NickSkier/abscli.git
cd abscli
```

```bash
meson setup builddir
```

```bash
ninja -C builddir
```

## Usage

```bash
./build/abscli
```

## Dependencies

This project relies on the following external libraries:

- **[nlohmann/json](https://github.com/nlohmann/json):**
- **[SQLite3](https://sqlite.org/):**
- **[CPR](https://github.com/libcpr/cpr):**
- **[keychain](https://github.com/hrantzsch/keychain):**

