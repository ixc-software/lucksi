TEMPLATE = lib

CONFIG += staticlib
CONFIG += precompile_header
CONFIG += create_prl

include(BaseConfig.pro)

# -------------------------------------------------------------------------------------
# contrib variables

include(Contrib/contrib.pri)
