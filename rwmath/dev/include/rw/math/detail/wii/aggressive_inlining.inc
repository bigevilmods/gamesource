// (c) Electronic Arts. All Rights Reserved.

// Getting Metrowerk's to inline code during a debug build can be quite
// painful.  Here are the options math uses to ensure inlining.  It is probably
// a good idea to use "pragma push" and "pragma pop" around these settings.

#pragma global_optimizer on
#pragma optimization_level 4
#pragma dont_inline off
#pragma always_inline on
#pragma auto_inline on
#pragma inline_depth(smart)
#pragma optimize_for_size off 
