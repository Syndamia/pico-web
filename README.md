# pico-web

Small client-server application.
The server receives a URL from a client and returns the appropriate page.

With Markdown pages the client adds special "rendering" like hyperlinks and bold text (via ANSI escape sequences).
The server is configured to send pages only in allowed directiores, and can handle multiple directiories (each connected to a URI's userinfo section) with custom error pages.

## Building and usage

Build the binaries:

```bash
make
```

You get two binaries, `server` and `browser`.

### Server

### Browser

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
On client connection, a new thread is started, which will handle parsing of URI and sending of page.

Client also creates a socket with which to connect to the server.
Upon receival of a page, it prints it on standard output.

### Page rendering

The client prints Markdown pages with special formatting.
Most elements use [ANSI escape sequences](https://en.wikipedia.org/wiki/ANSI_escape_code) to show text in different colors and format.

Anchors get even more special treatment: you can only see their contents (not the URL itself) and each has an index next to it.
By entering a number, you can follow the corresponding anchor.
Anchors are extracted with regular expression and the POSIX regex.h before the page is printed.
