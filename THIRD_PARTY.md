# Third-party components

## LavaXVM

- Source: <https://gitee.com/jacklee72/lavaxos/tree/master/LavaXVM>
- Vendored revision: `7c43a0f368dbb6bd55f793c5015377f5973a58f7`
- License: GPL-2.0; see `third_party/lavaxvm/LICENSE`.
- Local changes: the Nintendo DS hardware layer is replaced by a BBK 9588 BDA
  platform layer; blocking VM loops also pump the BBK platform.

The proprietary/non-commercial `LavaXOS` runtime, bundled `.lav` games and NDS
ROM files are deliberately not included in this project or in its BDA output.

## BBK 9588 BDA SDK

- Source: <https://github.com/HelloClyde/bbk9588-bda-sdk>
- Pinned revision: `d5d65917c19c8b8fb05a4da06eccfd0a12e90d97`
- License and notices are supplied by the SDK checkout.
