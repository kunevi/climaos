#include <stdint.h>
#include <stddef.h>
#include <stivale2.h>
#include <isr.h>
#include <string/printf.h>
#include <framebuffer/framebuffer.h>

static uint8_t stack[8192];

static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        // Identification constant defined in stivale2.h and the specification.
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        // If next is 0, it marks the end of the linked list of header tags.
        .next = 0
    },
    // The terminal header tag possesses a flags field, leave it as 0 for now
    // as it is unused.
    .flags = 0
};

static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = (uint64_t)&terminal_hdr_tag
    },
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};

__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    .tags = (uintptr_t)&framebuffer_hdr_tag
};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) {
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;) {
        if (current_tag == NULL) {
            return NULL;
        }

        if (current_tag->identifier == id) {
            return current_tag;
        }

        current_tag = (void *)current_tag->next;
    }
}

// The following will be our kernel's entry point.
void _start(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_framebuffer *fb_tag = (struct stivale2_struct_tag_framebuffer *)
        stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
    init_fb(fb_tag);
    printf("[OK] FB %ux%u\n", fb_tag->framebuffer_width, fb_tag->framebuffer_height);

    init_isr();
    printf("hold up chat");
    asm("int $0x20");
    printf("poggggg");

    for (;;) {
        asm ("hlt");
    }
}