#!/usr/bin/env python

# Returns the input command (like "AT") in hex string format, which can be used in the serial monitor.

import sys
command = sys.argv[1]

hexString = lambda l: '%02x' % ord(l)

command += "\r\n"
print(' '.join([hexString(l) for l in command]))
