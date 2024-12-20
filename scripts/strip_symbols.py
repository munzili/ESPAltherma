Import("env")

strip_tool = env['CC'].replace("gcc", "strip")
env.AddPostAction(
    "$BUILD_DIR/firmware.elf",
    env.VerboseAction(" ".join([
        strip_tool, "--strip-unneeded", "$BUILD_DIR/firmware.elf"
    ]), "Stripping symbols...")
)