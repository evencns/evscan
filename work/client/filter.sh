#!/bin/bash
cat data  | grep "*>" | awk '{print $2}' | grep -v "/32"  >subnet/subnetbgp.txt
