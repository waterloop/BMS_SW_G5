# BMS SW G5

Code repository for the Goose V Battery Management System Master Board.

# Installation

```bash
git clone https://github.com/waterloop/BMS_SW_G5.git
cd BMS_SW_G5
git submodule --init --recursive --remote
```


# Build

Start by building the `WLoopCan` submodule and then the BMS files

``` bash
cd /path/to/BMW_SW_G5
cd WLoopCAN
make master_bms
cd ..
make
```

# Intellisense

Supports the clangd language server: https://clangd.llvm.org

For VSCode see [here](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd).

# Updating `compile-commands.json`

Use `bear`:

```bash
# install bear...
sudo apt install bear

bear make
```

