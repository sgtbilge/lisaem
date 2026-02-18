#include "LisaEmCore.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// Targeted extern declarations to avoid including vars.h in the header
extern "C" {
    typedef uint8_t uint8;
    typedef uint16_t uint16;
    typedef uint32_t uint32;
    typedef int64_t XTIMER;

    extern uint8 *lisaram;
    extern uint32 maxlisaram;
    extern uint32 minlisaram;
    extern uint32 TWOMEGMLIM;
    extern uint32 videolatchaddress;
    extern uint32 videolatch;
    extern uint32 lastvideolatch;
    extern uint32 lastvideolatchaddress;
    extern XTIMER cpu68k_clocks;
    extern uint32 pc24;
    extern int running;
    extern uint8 floppy_iorom;
    extern int romless;
    extern uint8 eparity[256];
    extern int segment1, segment2, start;
    extern void* scc_a_port;
    extern void* scc_b_port;
    extern int scc_a_IW, scc_b_IW;
    extern int serial_a, serial_b;
    extern uint8 serialnum[8];
    extern int serialnumshiftcount, serialnumshift;
    extern uint8 serialnum240[16];
    extern int softmemerror, harderror, videoirq, bustimeout, videobit;
    extern int lisa_vid_size_x, lisa_vid_size_y, lisa_vid_size_xbytes;
    extern int has_lisa_xl_screenmod;
    extern uint8 dualparallelrom[2048];
    extern int videoramdirty;
    extern int macworks4mb;

    // Subsystem init functions
    void init_cops();
    void init_IRQ();
    void init_vias();
    void init_Profiles();
    void init_lisa_mmu();
    void init_ipct_allocator();
    void cpu68k_init();
    void cpu68k_reset();
    void init_floppy(long iorom);
    void disconnect_serial(int port);
    void initialize_scc(int port);
    void reg68k_sanity_check_bitorder();
    uint8 evenparity(uint8 b);
    void set_keyboard_id(int id);
    int read_dtc_rom(char *filename, uint8 *ROM);
    int read_split_rom(char *filename, uint8 *ROMMX);
    int read_rom(char *filename, uint8 *ROMMX);
    int checkromchksum();
    void fixromchk();
    int romless_boot(int pick);
    void vidfixromchk(uint8 *sn);
    void LisaRedrawPixels();
    void disable_vidram();
    void storeword(uint32 addr, uint16 data);
    
    // Core helpers
    void seek_mouse_event();
    int32_t reg68k_external_execute(int32_t clocks);
}

int romlessboot_pick();

LisaEmCore::LisaEmCore(LisaEmUI* ui) 
    : m_ui(ui), m_running(false), m_paused(false), m_lisaram_ptr(nullptr) {
}

LisaEmCore::~LisaEmCore() {
    Shutdown();
}

int LisaEmCore::Initialize(const LisaEmConfig& config) {
    m_config = config;
    m_ui->SetStatus("Initializing Emulation Core...");

    // Ported from initialize_all_subsystems
    
    // parity cache
    for (uint16_t i=0; i<256; i++) eparity[i]=evenparity((uint8) i);

    segment1=0; segment2=0; start=1;
    scc_a_port=NULL; scc_b_port=NULL;
    scc_a_IW=-1; scc_b_IW=-1;
    serial_a=0; serial_b=0;

    floppy_iorom = (uint8)m_config.iorom;
    init_floppy(m_config.iorom);

    for(int i=0; i<8; i++) serialnum[i]=0;
    serialnumshiftcount=0; serialnumshift=0;

    init_cops();
    init_IRQ();
    init_vias();
    init_Profiles();

    // RAM allocation
    switch(m_config.mymaxlisaram)
    {
       case 512  : maxlisaram=0x100000;  minlisaram=0x080000;  break;
       case 1024 : maxlisaram=0x180000;  minlisaram=0x080000;  break;
       case 1536 : maxlisaram=0x200000;  minlisaram=0x080000;  break;
       default:    maxlisaram=0x200000;  minlisaram=0x080000;  break;
    }

    TWOMEGMLIM=maxlisaram-1;
    videolatchaddress=maxlisaram-0x10000;
    videolatch=(maxlisaram>>15)-1;
    lastvideolatch=videolatch;  lastvideolatchaddress=videolatchaddress;

    if (m_lisaram_ptr) free(m_lisaram_ptr);
    m_lisaram_ptr = (uint8_t *) malloc( (macworks4mb ? 8 : 2) * 1024 * 1024 + 1024 );
    if (!m_lisaram_ptr) {
        m_ui->Error("Could not allocate Lisa RAM", "Out of Memory");
        return 23;
    }
    lisaram = m_lisaram_ptr;
    memset(m_lisaram_ptr, 0xff, 2*1024*1024+511);

    if (m_config.kbid) set_keyboard_id(m_config.kbid); else set_keyboard_id(-1);

    // Serial number parsing (simplified for now)
    memset(serialnum240, 0, 16);
    vidfixromchk(serialnum240);

    disconnect_serial(0);
    disconnect_serial(1);
    initialize_scc(0);

    softmemerror=0; harderror=0; videoirq=0; bustimeout=0; videobit=0;
    reg68k_sanity_check_bitorder();

    cpu68k_init();
    init_lisa_mmu();
    init_ipct_allocator();

    // ROM loading
    romless = 0;
    if (!m_config.rompath.empty()) {
        char path[1024];
        strncpy(path, m_config.rompath.c_str(), 1023);
        if (read_dtc_rom(path, m_lisaram_ptr) == 0 ||
            read_split_rom(path, m_lisaram_ptr) == 0 ||
            read_rom(path, m_lisaram_ptr) == 0) {
            fixromchk();
        } else {
            romless = 1;
        }
    } else {
        romless = 1;
    }

    if (romless) {
        int pick = romlessboot_pick();
        if (pick < 0) return -2;
        romless_boot(pick);
    }

    disable_vidram();
    videoramdirty = 32768;
    LisaRedrawPixels();
    cpu68k_reset();

    m_ui->SetStatus("Core Initialized");
    return 0;
}

int LisaEmCore::EmulateLoop(uint32_t now) {
    if (!m_running || m_paused) return 0;
    // Execution slice logic to be moved here
    return 0;
}

void LisaEmCore::Shutdown() {
    if (m_lisaram_ptr) {
        free(m_lisaram_ptr);
        m_lisaram_ptr = nullptr;
        lisaram = nullptr;
    }
    m_running = false;
}

void LisaEmCore::PowerOn() {
    m_running = true;
    m_paused = false;
}

void LisaEmCore::PowerOff() {
    m_running = false;
    m_ui->OnPowerOff();
}

void LisaEmCore::Reset() {
    cpu68k_reset();
    m_ui->OnReboot();
}

void LisaEmCore::Pause(bool paused) {
    m_paused = paused;
}
