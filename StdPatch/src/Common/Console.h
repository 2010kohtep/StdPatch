#pragma once

void DbgTrace(const char *pszFmt, ...);

void FailedToFind(const char *pszName);
void FailedToPatch(const char *pszName);
void FailedToHook(const char *pszName);