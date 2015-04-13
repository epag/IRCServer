#!/usr/bin/expect

spawn telnet data.cs.purdue.edu 2132
expect "'^]'."
send "SEND-MESSAGE peter spider room1 HELLO"
