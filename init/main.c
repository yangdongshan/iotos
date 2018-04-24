


int main()
{
    arch_init();

    board_init();

    os_init();

    user_main();

    thread_become_idle();

    return 0;
}
