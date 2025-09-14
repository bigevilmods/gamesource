// (c) Electronic Arts. All Rights Reserved.

#ifdef APT_PROFILE_ENABLED

void AptProfileBegin(uint32_t id, const char *name)
{
	gAptFuncs.pfnProfilerStartSample( name );
}
void AptProfileEnd(uint32_t id)
{
	gAptFuncs.pfnProfilerStopSample();
}

#endif