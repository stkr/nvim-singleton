
rem set PATH=%PATH%;c:\Program Files\CMake\bin;c:\Program Files\make\bin;c:\Program Files\ninja;c:\Program Files\ARMCompiler6.15\bin
rem set ARMLMD_LICENSE_FILE=9613@liv0058.nxdi.us-cdc01.nxp.com:9613@liv0042.nxdi.nl-cdc01.nxp.com,9613@liv0043.nxdi.nl-cdc01.nxp.com,9613@liv0044.nxdi.nl-cdc01.nxp.com:8224@liv0023.nxdi.us-cdc01.nxp.com:9613@liv0048.nxdi.nl-cdc01.nxp.com,9613@liv0049.nxdi.nl-cdc01.nxp.com,9613@liv0050.nxdi.nl-cdc01.nxp.com:9613@liv6007.in-blr01.nxp.com,9613@liv6008.in-blr01.nxp.com,9613@liv6009.in-blr01.nxp.com:9613@liv8008.nxdi.in-nda02.nxp.com,9613@liv8009.nxdi.in-nda02.nxp.com,9613@liv8010.nxdi.in-nda02.nxp.com:9613@liv6004.in-blr01.nxp.com,9613@liv6005.in-blr01.nxp.com,9613@liv6006.in-blr01.nxp.com:9613@liv7001.cn-sha01.nxp.com,9613@liv7002.cn-sha01.nxp.com,9613@liv7003.cn-sha01.nxp.com
rem set BUILD_CONFIGURATION=debug
rem set BUILD_TARGET=cc
rem 
rem cmake                                                                                     ^
rem     -B %BUILD_TARGET%                                                                     ^
rem     -DCMAKE_EXPORT_COMPILE_COMMANDS=1                                                     ^
rem     -DMCUBOOT_PLATFORM=RW610                                                              ^
rem     -DMCUBOOT_SIM_TARGET=RTL                                                              ^
rem     -DMCUBOOT_TOOLCHAIN_FILE=toolchain_ARMCLANG.cmake                                     ^
rem     -DCMAKE_BUILD_TYPE=%BUILD_CONFIGURATION%                                              ^
rem     -DMCUBOOT_CMSIS_DIR="c:\Users\nxp29037\AppData\Local\Arm\Packs\ARM\CMSIS\5.7.0\CMSIS" ^
rem     -GNinja

rem start "" "c:\Program Files\Neovim\bin\nvim-qt.exe" -- -S project.vim
start "" "C:\Program Files\Alacritty\alacritty.exe" -o window.dimensions.columns=200 -o window.dimensions.lines=55 -o window.position.x=10 -o window.position.y=10 -e "nvim -S .project.vim""
