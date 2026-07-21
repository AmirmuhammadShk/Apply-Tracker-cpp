# Apply Tracker C++

Apply Tracker C++ is a small application for tracking job applications in Google Sheets.

The project provides:

* A CLI to add and list job applications
* A REST API to add and retrieve applications
* Google Sheets integration using a service account
* A modular C++ architecture shared between the CLI and API

## Install, Build and Run

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  libcurl4-openssl-dev \
  libssl-dev \
  nlohmann-json3-dev \
  libdrogon-dev \
  libjsoncpp-dev \
  libpq-dev \
  libsqlite3-dev \
  default-libmysqlclient-dev \
  libhiredis-dev
```

### Fedora / Red Hat

```bash
sudo dnf install -y \
  gcc-c++ \
  cmake \
  make \
  libcurl-devel \
  openssl-devel \
  nlohmann-json-devel \
  jsoncpp-devel \
  libpq-devel \
  sqlite-devel \
  mariadb-connector-c-devel \
  hiredis-devel \
  drogon-devel
```

### Build

```bash
cmake -S . -B build
cmake --build build -j
```

### Run CLI

```bash
./build/apply-tracker-cli
```

CLI commands:

```text
1  Add application
2  List applications
0  Exit
```

### Run API

```bash
./build/apply-tracker-api
```

### Test API

```bash
curl http://localhost:8080/health
```

```bash
curl http://localhost:8080/applications
```

```bash
curl -X POST http://localhost:8080/applications \
  -H "Content-Type: application/json" \
  -d '{
    "role": "C++ Software Engineer",
    "company": "Example Company",
    "job_description": "C++ development role",
    "location": "Cambridge",
    "type": "Permanent",
    "working_type": "Hybrid",
    "salary": "£70,000",
    "status": "Applied"
  }'
```

## Why I Built It

I am currently looking for C++ software engineering roles in the UK.

I created this project for two main reasons:

1. To keep my job applications organised in one Google Sheet.
2. To practise modern C++ by building a real project with REST APIs, authentication, HTTP requests, CMake, and modular architecture.

The project also helps me practise writing reusable components that can support multiple interfaces, such as a command-line application and an HTTP API.

