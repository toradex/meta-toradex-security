SRC_URI:append = "\
    file://dm-crypt.cfg \
    file://trusted-keys.cfg \
    file://${TDX_ENC_KEY_BACKEND}.cfg \
"
