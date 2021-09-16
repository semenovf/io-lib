################################################################################
# Copyright (c) 2019-2021 Vladislav Trifochkin
#
# This file is part of [io-lib](https://github.com/semenovf/io-lib) library.
#
# Changelog:
#      2021.09.10 Initial version.
################################################################################
if (ENABLE_LIBZMQ)
    add_subdirectory(lazypirate)
endif(ENABLE_LIBZMQ)

if (ENABLE_NANOMSG)
    add_subdirectory(network_discoverer)
endif(ENABLE_NANOMSG)
