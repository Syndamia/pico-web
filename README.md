# pico-web

Small client-server network socket application.
The server receives a URL from a client and returns the appropriate page.

## DevSecOps lifecycle

### 1. Plan

With [GitHub issues](https://github.com/Syndamia/pico-web/issues) and/or [GitHub pull requests](https://github.com/Syndamia/pico-web/pulls) modifications to the project are started and discussed

### 2. Code

Our branching strategy is a "feature workflow with stable branches", meaning:

- a feature branch is created (for each issue)
- after the feature is completed in the branch, it is merged into the `dev` branch
- after enough time has passed, the `dev` branch is merged into the `main` branch

Feature branches don't require special naming (though obviously can't be called `dev` or `main`).  
Code can be added to `dev` only via pull requests from feature branches.  
Code can be added to `main` only via pull requests from `dev`.  

Merge requests must always be approved by a contributor and `dev` merge requests to `main` must always have one of these labels: `release:major`, `release:minor` or `release:patch`.

These constraints are checked with workflows.

### 3. Continuous Integration: Build, Test, Security

On each push to feature branches and `dev` we execute the "cd" pipeline, during which we do:

- Code testing:
  - unit tests
  - [clang](https://clang.llvm.org/)'s `--analyze` static analysis
- SAST, with multiple different tools:
  - [flawfinder](https://dwheeler.com/flawfinder/)'s security analysis
  - [SonarCloud](https://www.sonarsource.com/products/sonarcloud/) source code analysis *(automatic, not from workflow)*
<!--
- SCA: https://github.com/multilang-depends/depends
-->
- Application build
- Container security testing (both production and dev containers):
  - [Trivy](https://trivy.dev/) docker container (binary) analysis

### 4. Continuous Deployment: Release, Deploy

On each successful merge request to `dev`,

- a development docker image is released to [dockerhub](https://hub.docker.com/r/syndamia/pico-web-dev)

On each successful merge request to `main`,

- the production docker image is released to [dockerhub](https://hub.docker.com/r/syndamia/pico-web), and it is scanned with [Trivy](https://trivy.dev/)
- a [GitHub release](https://github.com/Syndamia/pico-web/releases) is created, according to the pull request label, and
- the kubernetes cluster is deployed with [minkube](https://minikube.sigs.k8s.io/docs/) in the pipeline

### Non-code solutions

- [SonarCloud](https://www.sonarsource.com/products/sonarcloud/), since for C it would require a complete compilation toolchain remake
- Certain branch protections/requirements, since they can only be configure with GitHub website settings

## Project details

You get two binaries, `server` and `browser`.

URIs (URLs) are in the form `userinfo@address:portPATH`, where `address` and `port` could be skipped.
The userinfo section is analogous to subdomains in normal web applications.

It's assumed that all pages are written in Markdown so the client adds special "rendering" like hyperlinks and bold text (via ANSI escape sequences).
The server is configured to send pages only in allowed directories, and can handle multiple directories with custom error pages.

### Browser

The `browser` program takes no arguments.
Above every page you can see it's URI.
When ran it displays a blank page by default.

Markdown anchors are handled in a very special manner: you don't see the actual address, only the "name", and before every name is appended a number.
This number is a unique index, referencing the given anchor.

Below the last page you can write commands for the browser.
Commands come in four "flavours":

- Full URI, which will be the next page to display.
  You may omit the host or port, the last values will be taken (which by default are 127.0.0.1 and 8080).
  For example: `hello@/`.

- Relative path, which as the name suggests, is just a file path which will be requested on the previous username, address and host.
  For example: `/index.md`.

- Number, which is one of the aforementioned prepended numbers to anchors.
  You would "follow" the anchor, effectively navigating to it's stored address.

- Column command, a line which begins with the letter `:` and is then *immediately* followed by the name of the command.
  There are only four command names: `quit`, `q`, `exit` and `e`, all exiting from the program.
  For example: `:q`.

### Server

It takes any number of comma-separated strings.

By default the server is available on address 127.0.0.1 and port 8080.
If the first argument contains only one comma (so two items), then it's parsed as an address-port pair (for example `127.0.0.2,8081`) which overrides the defaults.

If there are two commas (three items; and that's the required amount for any argument after the first), then it contains an access username, files root and error file.
All files under the "files root" directory will be available on the userinfo "username".
If the server cannot find a requested file, then the error file is returned (whose path is relative to the files root).

*Note:* A quirk with Markdown anchors is that relative paths are always relative to the files root, not the current directory.

For example, if you want to share a folder `~/Documents` on username `mynotes` with an error file `./error.md` (again, this path is relative to `~/Documents`), then the corresponding argument to `server` would be `mynotes,~/Documents,./error.md`.

When running you'll see logs, related to all connections with the server.

You can also type commands, similarly to [browser](#browser), the first character on the line must be `:`, followed immediately by the command name.
There are currently 3 distinct commands:

- `help`, `h` and `?`, which print all available commands
- `vhosts`, which prints out all shared folders, alongside their associated userinfo and error file
- `quit`, `q`, `exit` and `e`, which exit out of the program

### Demo

This project comes with a tiny assortment of pages.
This is a sample input and output of both programs, showing all of their capabilities (browser does add some special coloring in terminals, which can't be shown in this file), where shell commands are prepended with `$ ` and user input with `> `:

```
$ ./server '127.0.0.1,8081` `demo,./demo,./demo/page.md`
Listening on 127.0.0.1:8081
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/
[127.0.0.1@4] Serving ./demo/index.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/page.md
[127.0.0.1@4] Serving ./demo/page.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/
[127.0.0.1@4] Serving ./demo/index.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/dir
[127.0.0.1@4] Serving ./demo/dir/index.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/dir/page.md
[127.0.0.1@4] Serving ./demo/dir/page.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/page.md
[127.0.0.1@4] Serving ./demo/page.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/
[127.0.0.1@4] Serving ./demo/index.md
[127.0.0.1@4] Served!
[127.0.0.1@4] Connected successfully!
[127.0.0.1@4] Requested demo@/dira
[127.0.0.1@4] Error opening ./demo/dira
[127.0.0.1@4] Serving ./demo/page.md
[127.0.0.1@4] Served!
> :help
help,h,?    Prints this message
vhosts      Prints all registered virtual hosts
quit,exit,q,e   Exits the program
> :vhosts
Name: "demo" Root dir: "./demo" Error file: "page.md"
> :quit
Exiting...
```

```
$ ./browser
blank

> demo@127.0.0.1:8081/   
demo@127.0.0.1:8081/
<< Index page >>

Go to: 0page, 1dir
> 0
demo@127.0.0.1:8081/page.md
<< Page >>

Go to: 0Index
> 0
demo@127.0.0.1:8081/
<< Index page >>

Go to: 0page, 1dir
> 1
demo@127.0.0.1:8081./dir
[[ Index of dir ]]

Go to: 0Index, 1Page
> 1
demo@127.0.0.1:8081/dir/page.md
[[ Page in dir ]]

Go to: 0Index
> demo@/page.md
demo@/page.md
<< Page >>

Go to: 0Index
> /
demo@/
<< Index page >>

Go to: 0page, 1dir
> demo@127.0.0.1:8081/dira
<< Page >>

Go to: 0Index
> blank
blank

> :quit
```

## Source code information

### File hierarchy

- `browser.c` - Main browser source code, handles communication with server
- `browser-cli.c` - Handles rendering of pages (printing to stdout) and user interface (user input)

- `server.c` - Main server source code, handles communication with client
- `server-connection.c` - Parses received information from client and sends information back to client
- `server-cli.c` - Handles user interface (user input)

- `util.c` - Various functions in use by the browser and server apps

### Networking

The server creates a socket on which it listens for incoming connections.
On client connection, a new child process is started, which will handle parsing of URI and sending of page.

Client also creates a socket with which to connect to the server.
When receiving a page, it prints it on standard output.

Relevant (networking) code for the server is located in `server.c` as well as `on_connection` function in `server-connection.c`.
For the browser it's all in `browser.c`.

### Page rendering

The client prints Markdown pages with special formatting.
Most elements use [ANSI escape sequences](https://en.wikipedia.org/wiki/ANSI_escape_code) to show text in different colors and format.

Special Markdown syntax (anchors, ...) is parsed with regular expressions via the POSIX regex.h.
