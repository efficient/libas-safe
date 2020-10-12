b main
  comm
    en 2
    en 3
    en 4
    c
  end
set br p on
b malloc
  comm
    dis 2
    p $arg = $rdi
    f 1
    b +1
      comm
        p printf("TCB    %#x-%#x\n", $rax, $rax + $arg)
        c
      end
    c
  end
dis $bpnum
b memmove-vec-unaligned-erms.S:mempcpy
  comm
    p printf(".tdata %#x-%#x\n", $rdi, $rdi + $rdx)
    c
  end
dis $bpnum
b memset-vec-unaligned-erms.S:memset
  comm
    p printf(".tbss  %#x-%#x\n", $rdi, $rdi + $rdx)
    c
  end
dis $bpnum
b _dl_deallocate_tls
  comm
    dis 3
    dis 4
    c
  end
