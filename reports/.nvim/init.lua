vim.api.nvim_create_autocmd("User", {
  pattern = "VimtexEventCompileSuccess",
  callback = function()
    vim.fn.jobstart("texcount -inc -v -sum main.tex | grep 'Sum count' | tail -n 1 | awk '{print $3}' > wordcount.tex", { detach = true })
  end,
})
