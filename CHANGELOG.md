# Changelog

All notable changes to this project will be documented in this file.

The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased] - yyyy-mm-dd

Here are limitations of the current implementation that I know of.

### To do

- UDP Functionality
- sweep inactive clients between socket events

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
