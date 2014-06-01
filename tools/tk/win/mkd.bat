@echo off
rem RCS: @(#) $Id: mkd.bat,v 1.4 2003/03/19 23:02:11 cagney Exp $

if exist %1\nul goto end

md %1
if errorlevel 1 goto end

echo Created directory %1

:end



