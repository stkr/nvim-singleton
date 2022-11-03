" hi LineNr guibg=#268bd2 guifg=#eee8d5
set sessionoptions=buffers

" Language server protocol {{{

if (has('nvim'))
    " Enable clangd
    lua << EOF
    
    require("packer.load")({'nvim-cmp'}, {}, _G.packer_plugins)
    require'lspconfig'.clangd.setup{
        -- cmd = { "c:\\Program Files\\LLVM_14.0.0\\bin\\clangd.exe", "--background-index", "--compile-commands-dir=RTL_Debug" },
        cmd = { "c:\\Program Files\\LLVM_14.0.0\\bin\\clangd.exe" },
        capabilities = require('cmp_nvim_lsp').update_capabilities(vim.lsp.protocol.make_client_capabilities())
    };
EOF
endif
" }}}

source .session.vim
