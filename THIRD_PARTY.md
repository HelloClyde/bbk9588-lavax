# Third-party components

## LavaXVM

- Source: <https://gitee.com/jacklee72/lavaxos/tree/master/LavaXVM>
- Vendored revision: `7c43a0f368dbb6bd55f793c5015377f5973a58f7`
- License: GPL-2.0; see `third_party/lavaxvm/LICENSE`.
- Local changes: the Nintendo DS hardware layer is replaced by a BBK 9588 BDA
  platform layer; blocking VM loops also pump the BBK platform.

## LavaXOS runtime package

- Source: <https://gitee.com/jacklee72/lavaxos/tree/master/LavaXOS>
- Packaged revision: `7c43a0f368dbb6bd55f793c5015377f5973a58f7`
- License: GPL-2.0; the upstream `LICENSE` is included in `LavaXOS.zip`.
- Packaging changes: `LavaXOS/_NDS` and every `.nds` file are excluded. The
  remaining `System`, `LAVA`, `PROGRAM` and `DOCUMENT` content is copied without
  modification.

The runtime is distributed as a separate release asset and is not embedded in
the BDA. Other ROM files are not included.

## BBK 9588 BDA SDK

- Source: <https://github.com/HelloClyde/bbk9588-bda-sdk>
- Pinned revision: `d5d65917c19c8b8fb05a4da06eccfd0a12e90d97`
- License and notices are supplied by the SDK checkout.
