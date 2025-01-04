" feel free to load the conf for this project by adding these 3 lines to your vim config file, then loading vim in this folder
"if filereadable(".vimrc")
"    so .vimrc
"endif

" tab = 4 spaces
set tabstop=4
set expandtab
set shiftwidth=4
" build/run commands
if has('mac')
    "let &makeprg='xcodebuild -project ../../wasteladns.xcodeproj -scheme app-macos'
    let &makeprg='clang++ -x objective-c++ -std=gnu++14 -fobjc-arc -D__MACOS=1 -D__GL33=1 -framework Cocoa -framework IOKit main.cpp -o ../../bin/Debug/app-macos'
    noremap <leader>rr :!cd ./../../bin/Debug && ./app-macos &<CR>
elseif has ('win64')
    " build (only works if cmd has the rigth env variables)
    " 	(use bats in C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Visual Studio 2022\Visual Studio Tools\VC)
    "set makeprg=msbuild\ ..\..\wasteladns.sln\ /nologo\ /v:q\ /property:GenerateFullPaths=true
    let &makeprg='cl /Fe"..\..\bin\Debug\app-dx11.exe" /Fo..\..\app-dx11.dir\Debug\ main.cpp "user32.lib" /D "__WIN64=1" /D "__DX11=1"'
    "noremap <leader>bd :set makeprg=msbuild\ ..\..\app-dx11.vcxproj\ /nologo\ /v:q\ /property:GenerateFullPaths=true<CR>
    "noremap <leader>bg :set makeprg=msbuild\ ..\..\app-gl33.vcxproj\ /nologo\ /v:q\ /property:GenerateFullPaths=true<CR>
    noremap <leader>bd :let &makeprg='cl /Fe"..\..\bin\Debug\app-dx11.exe" /Fo..\..\app-dx11.dir\Debug\ main.cpp "user32.lib" /D "__WIN64=1" /D "__DX11=1"'<CR>
    noremap <leader>bg :let &makeprg='cl /Fe"..\..\bin\Debug\app-dx11.exe" /Fo..\..\app-dx11.dir\Debug\ main.cpp "user32.lib" "gdi32.lib" /D "__WIN64=1" /D "__GL33=1"'<CR>
    noremap <leader>rd :!cd .\..\..\bin\Debug && START /B .\app-dx11.exe<CR>
    noremap <leader>rg :!cd .\..\..\bin\Debug && START /B .\app-gl33.exe<CR>
endif
" save all buffers, then compile
noremap <leader>bb :wa\|:make<CR>
