@echo off
rem RCS: @(#) $Id: mkd.bat,v 1.3 2012/04/18 00:37:11 kevinb Exp $

if exist %1\nul goto end

md %1
if errorlevel 1 goto end

echo Created directory %1

:end



