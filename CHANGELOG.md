# Changelog

All notable changes to this project will be documented in this file.

The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased] - yyyy-mm-dd

Here are limitations of the current implementation that I know of.

### To do

- UDP Functionality
- Error state handling (there is none yet)

## [0.4.0] - 2024-04-22

Version as of commit `483990bbc0efb4ee4ff98ddf1a992db10d4224c5`

### Added

- Ported and updated TCP message parsing and rendering from the first project
- Functionality to handle multiple TCP clients, TCP send + recv
- Broadcasting messages
- Broadcasting clients joining/leaving
- Functionality for JOIN messages and having multiple channels
