require ${@oe.utils.conditional('TDX_K3_HSSE_ENABLE', '1', 'ti-k3-secdev-hsse.inc', '', d)}
