# Arguru Aodix

[Aodix](https://web.archive.org/web/20070819041559/http://www.aodix.com/pageaodixv4.html) was a music sequencer program for Windows developed by Arguru Software. It had a unique interface combining the features of [trackers](https://en.wikipedia.org/wiki/Music_tracker) and [sequencers](https://en.wikipedia.org/wiki/Music_sequencer), as well as a modular wiring system for VSTs.

The developer of Aodix, [Juan Antonio Arguelles ("Arguru")](https://en.wikipedia.org/wiki/Juan_Antonio_Arguelles_Rius), passed away in 2007. The final version of Aodix, v4.2.0.1, was released as freeware, however the source code was not published at the time.

I received permission from Zafer Kantar, who worked with Arguru on Aodix, to publish this code under an open source license. The code is from a backup dated November 5th 2005, which corresponds to a beta version of v4.2.0.0 (preserved on the `backup` branch). I attemped to reconstruct the changes made between this beta version and the final release, based on the decompiled executable and the [changelog](https://github.com/arguru-archive/aodix/blob/v4.2.0.1/changelog.txt). The reconstructed code is on the `v4.2.0.1` branch.

This repository will serve as an archive only. I plan to continue development of Aodix on my [personal fork](https://github.com/vanjac/aodix).

## Building

Aodix can be built using Visual Studio 2003 (run on Windows XP for best results). Instead of MSVC, Aodix was originally compiled with the [Intel C++ Compiler](https://en.wikipedia.org/wiki/Intel_C%2B%2B_Compiler). Unfortunately I wasn't able to find the correct version of this compiler used by Arguru; I used version 7.1.029 for the reconstruction, which produced similar but not identical results. MSVC can still be used as a substitute.

This repository does not include the [ASIO SDK](https://www.steinberg.net/developers/) (version 2.0) or the VST SDK (version 2.3, no longer available), since they are proprietary.
