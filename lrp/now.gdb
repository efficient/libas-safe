set br p on
break _dl_relocate_object
  commands
    p ((struct link_map *) $rdi)->l_name
  end
