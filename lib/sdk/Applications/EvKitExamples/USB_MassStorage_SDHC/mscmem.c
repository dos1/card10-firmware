#include "mscmem.h"
#include <string.h>
#include <stdio.h>
#include "mx25.h"
#include "gpio.h"
#include "mxc_assert.h"
#include "sdhc_lib.h"

/***** Definitions *****/

#define LBA_SIZE                    512         /* Size of "logical blocks" in bytes */
#define LBA_SIZE_SHIFT              9           /* The shift value used to convert between addresses and block numbers */
#define SECTOR_SIZE                 32768
#define SECTOR_SIZE_SHIFT           15
#define LBA_PER_SECTOR              (1<<(SECTOR_SIZE_SHIFT - LBA_SIZE_SHIFT))      /* 512 byte blocks in each sector */

#define NUM_DISK_BUFS               8
#if (NUM_DISK_BUFS < 2)
#error NUM_DISK_BUFS must be > 1
#endif

uint64_t sz_eblk;

#define INIT_CARD_RETRIES 10

/***** Global Data *****/

extern uint32_t systemTicks;
/***** File Scope Variables *****/

static int initialized = 0;
static int running = 0;

/***** File Scope Variables *****/
typedef struct {
    unsigned int valid;
    unsigned int dirty;
    unsigned int age;
    uint32_t sectorNum;
    uint8_t sector[SECTOR_SIZE];
} disk_buf_t;

static disk_buf_t diskbuf[NUM_DISK_BUFS];
static unsigned int async_write_pending = 0;
static unsigned int async_buf_idx;

/***** Function Prototypes *****/
static uint32_t getSectorNum(uint32_t lba);
static uint32_t getSectorAddr(uint32_t lba);
static int getSector(uint32_t num);

/******************************************************************************/
static uint32_t getSectorNum(uint32_t lba)
{
    return lba >> (SECTOR_SIZE_SHIFT - LBA_SIZE_SHIFT);
}

/******************************************************************************/
static uint32_t getSectorAddr(uint32_t lba)
{
    return (lba & (LBA_PER_SECTOR - 1)) << LBA_SIZE_SHIFT;
}

static void writeCB(int error)
{
    MXC_ASSERT(error == E_NO_ERROR);
    if (async_write_pending && (async_buf_idx < NUM_DISK_BUFS)) 
    {
        diskbuf[async_buf_idx].dirty = 0;
    } 
    else 
    {
        MXC_ASSERT_FAIL();
    }

    async_write_pending = 0;
}

static unsigned int tick = 0;

/******************************************************************************/
static int getSector(uint32_t num)
{
#if 1
    unsigned int idx, oldest, tmp;
    int err;

    /* Advance time */
    tick++;
    oldest = 0;
    
    /* See if requested sector is cached */
    for (idx = 0; idx < NUM_DISK_BUFS; idx++) 
    {
        if (diskbuf[idx].valid) 
        {
            if (diskbuf[idx].age < diskbuf[oldest].age) 
            {
                oldest = idx;
            }
            if (diskbuf[idx].sectorNum == num) 
            {
                /* Found it */
                diskbuf[idx].age = tick;
                break;
            } 
        }
        else
        {
            /* Wait for any async writes to complete */
            while (async_write_pending) 
            {
                SDHC_Lib_Async_Handler();
            }
            /* Use this empty buffer for new sector */
            err = SDHC_Lib_Read(diskbuf[idx].sector, num*LBA_PER_SECTOR, SECTOR_SIZE >> LBA_SIZE_SHIFT, SDHC_LIB_QUAD_DATA);
            if (err != E_NO_ERROR) 
            {
                MXC_ASSERT_FAIL();
                return -1;
            }
            diskbuf[idx].sectorNum = num;
            diskbuf[idx].dirty = 0;
            diskbuf[idx].age = tick;
            diskbuf[idx].valid = 1;
            break;
        }
    }

    if (idx < NUM_DISK_BUFS) 
    {
        /* Sector found */
        oldest = 0;
        for (tmp = 0; tmp < NUM_DISK_BUFS; tmp++) 
        {
            if ((tmp != idx) && (diskbuf[tmp].age < diskbuf[oldest].age)) 
            {
                /* Oldest buffer, but not the one currently in use */
                oldest = tmp;
            }
        }
        if(!async_write_pending)
        {
            /* Asynchronously flush, if dirty and sufficiently aged */
            if (diskbuf[oldest].valid && diskbuf[oldest].dirty && diskbuf[oldest].age < (tick - (NUM_DISK_BUFS/2)) ) 
            {
                if (SDHC_Lib_WriteAsync(diskbuf[oldest].sectorNum * LBA_PER_SECTOR, (void *)diskbuf[oldest].sector, SECTOR_SIZE >> LBA_SIZE_SHIFT, SDHC_LIB_QUAD_DATA, writeCB) == E_NO_ERROR) 
                {
                    async_write_pending = 1;
                    async_buf_idx = oldest;
                } 
                else 
                {
                    MXC_ASSERT_FAIL();
                }
            }
        }
    }
    else 
    {
        /* Wait for any async writes to complete */
        while (async_write_pending) 
        {
            SDHC_Lib_Async_Handler();
        }


         /* No free sector found, replace oldest with requested sector */
        idx = oldest;
        if (diskbuf[idx].valid && diskbuf[idx].dirty) 
        {
            /* Synchronously flush to disk */
            err = SDHC_Lib_Write(diskbuf[idx].sectorNum * LBA_PER_SECTOR, (void *)diskbuf[idx].sector, SECTOR_SIZE >> LBA_SIZE_SHIFT, SDHC_LIB_QUAD_DATA);
            if (err != E_NO_ERROR) 
            {
                MXC_ASSERT_FAIL();
                return -1;
            }
        }
        /* Read in new sector */
        err = SDHC_Lib_Read(diskbuf[idx].sector, num*LBA_PER_SECTOR, SECTOR_SIZE >> LBA_SIZE_SHIFT, SDHC_LIB_QUAD_DATA);
        if (err != E_NO_ERROR) 
        {
            MXC_ASSERT_FAIL();
            return -1;
        }

        diskbuf[idx].sectorNum = num;
        diskbuf[idx].dirty = 0;
        diskbuf[idx].age = tick;
        diskbuf[idx].valid = 1;
    }

    return idx;
#else    
        int err = SDHC_Lib_Read(diskbuf[0].sector, num*LBA_PER_SECTOR, SECTOR_SIZE >> LBA_SIZE_SHIFT, SDHC_LIB_QUAD_DATA);
        if (err != E_NO_ERROR) 
        {
            MXC_ASSERT_FAIL();
            return -1;
        }
        return 0;
#endif        
}


/******************************************************************************/
int mscmem_init()
{
    if(!initialized) 
    {

        mxc_sdhc_csd_regs_t csd;
        const sys_cfg_sdhc_t sys_sdhc_cfg = NULL; /* No system specific configuration needed. */
        sdhc_cfg_t cfg;

        // Power Cycle the Card
        gpio_cfg_t SDPowerEnablePin = {PORT_1, PIN_12, GPIO_FUNC_OUT, GPIO_PAD_NONE};
        GPIO_Config(&SDPowerEnablePin);
        GPIO_OutSet(&SDPowerEnablePin);
//        mxc_delay(5000);
        GPIO_OutClr(&SDPowerEnablePin);

        // Set up Interupt    ???
        NVIC_EnableIRQ(SDHC_IRQn);

        // Initialize SDHC peripheral
        cfg.bus_voltage = SDHC_Bus_Voltage_3_3;
        cfg.block_gap = 0;
        cfg.clk_div = 0xB0; // Maximum divide ratio, frequency must be >= 400 kHz during Card Identification phase

        if(SDHC_Init(&cfg, &sys_sdhc_cfg) != E_NO_ERROR) 
        {
            printf("Unable to initialize SDHC driver.\n");
            return 1;
        }
        else
        {
            printf("sdhc_init success\r\n");
        }

        // wait for card to be inserted
        while (!SDHC_Card_Inserted());
        printf("Card inserted.\n");

        // set up card to get it ready for a transaction
        if (SDHC_Lib_InitCard(10) == E_NO_ERROR) 
        {
            printf("Card Initialized.\n");
        } 
        else 
        {
            printf("No card response! Remove card, reset EvKit, and try again.\n");
            return -1;
        }
        
        if (SDHC_Lib_Get_Card_Type() == CARD_SDHC) 
        {
            printf("Card type: SDHC\n");
        }
        else 
        {
            printf("Card type: MMC/eMMC\n");
        }

        printf("sdhc clock:%ld\n",SDHC_Get_Clock_Config());

        /* Configure for fastest possible clock, must not exceed 52 MHz for eMMC */
        if (SystemCoreClock > 96000000)  
        {
            printf("SystemCoreClock:%ld.\n",SystemCoreClock);
            printf("SD clock ratio (at card) 4:1\n");
            SDHC_Set_Clock_Config(1);
        }
        else 
        {
            printf("SD clock ratio (at card) 2:1\n");
            SDHC_Set_Clock_Config(0);
        }

        MXC_SDHC->clk_cn = 0x105;
    
        if ((SDHC_Lib_GetCSD(&csd)) == E_NO_ERROR) 
        {
            sz_eblk = SDHC_Lib_GetCapacity(&csd);
            initialized = 1;
            printf("%s:get csd success.  Size = %llu\n",__FUNCTION__, sz_eblk);
        }
        else
        {
            printf("%s:get csd error.\n",__FUNCTION__);
        }
    }

    return 0;
}

/******************************************************************************/
uint32_t mscmem_size(void)
{
    if(initialized)
    {
        return sz_eblk >> LBA_SIZE_SHIFT;
    }
    else
    {
        return 0;
    }
}

/******************************************************************************/
int mscmem_read(uint32_t lba, uint8_t* buffer)
{
    uint32_t addr;
    uint32_t sNum = getSectorNum(lba);
    int idx;

    idx = getSector(sNum);
    if (idx < 0) 
    {
        return 1;    // Failed to write/read from sd
    }

    addr = getSectorAddr(lba);    // Get the offset into the current sector
    memcpy(buffer, diskbuf[idx].sector + addr, LBA_SIZE);

    return 0;
}

/******************************************************************************/
int mscmem_write(uint32_t lba, uint8_t* buffer)
{
    uint32_t addr;
    uint32_t sNum = getSectorNum(lba);
    int idx;

    idx = getSector(sNum);
    if (idx < 0) 
    {
        return 1;    // Failed to write/read from sd
    }

    // Get the offset into the current sector
    addr = getSectorAddr(lba);

    memcpy(diskbuf[idx].sector + addr, buffer, LBA_SIZE);
    diskbuf[idx].dirty = 1;

    return 0;

}

/******************************************************************************/
int mscmem_start()
{
    if(!initialized) 
    {
        mscmem_init();
    }
    
    if(initialized) 
    {
        running = 1;
    }
    
    return !initialized;
}

/******************************************************************************/
int mscmem_stop()
{
#if 0
    // Flush the currently cached sector if necessary.
    if(getSector((0xFFFFFFFFUL))) {
        return 1;
    }
#endif

    running = 0;
    return 0;
}

/******************************************************************************/
int mscmem_ready()
{
    return running;
}

void SDHC_IRQHandler()
{
    SDHC_Lib_Async_Handler();
}