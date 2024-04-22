# Changelog

All notable changes to this project will be documented in this file.

The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased] - yyyy-mm-dd

Here are limitations of the current implementation that I know of.

### To do

- UDP retransmitting
- sweep inactive clients between socket events (`clientlist.c`)

## [0.9.0] - 2024-04-22

Version as of commit `e6e0c856573b13a4a126e90119674f1b5135e043`

UDP clients can now have identical experience
to TCP clients, just less reliable, because the server can not yet
retransmit messages when they are not confirmed.

### Added

- Ported and extended `udp_render` module from the first project
- Implemented `udp_parse` module to adapt for the common `msg` module
- Integrated the UDP interface

### Notes
- Everything about UDP shares the same logic as TCP, except for the function
that handles authenticating. Because UDP AUTH messages come to the common
welcome socket, it was difficult to adapt it to the common interface.
Therefore, I decided to re-implement it for UDP (compare `client_udp_auth`
and `client_process_message` in `client.c`)



## [0.5.0] - 2024-04-22

Version as of commit `cf5ea6e3c62f5fa9a45c45766e3cbed1b4be2e55`

### Added

- Error state handling

### Fixed

- Better TCP parsing - no leading/trailing invalid characters are allowed
now

## [0.4.0] - 2024-04-22

Version as of commit `483990bbc0efb4ee4ff98ddf1a992db10d4224c5`

### Added

- Ported and updated TCP message parsing and rendering from the first project
- Functionality to handle multiple TCP clients, TCP send + recv
- Broadcasting messages
- Broadcasting clients joining/leaving
- Functionality for JOIN messages and having multiple channels
