#!/bin/sh

progbase="birnlexvis"
bindir=$(dirname "$0")
parentdir="${bindir}/.."
extjardir="./extjars"
datadir="./data"
bindir="./bin"

cd $parentdir

binjars="${bindir}/${progbase}-support.jar;${bindir}/${progbase}.jar"
extjars="${extjardir}/json.jar;${extjardir}/prefuse.jar;${extjardir}/jython.jar"

CLASSPATH="${binjars};${extjars}"; export CLASSPATH

exec  java  "${progbase}" "${datadir}/*.json"




