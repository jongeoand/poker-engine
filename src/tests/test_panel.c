void test_panel(void) {
    printf("=== TextPanel ===\n\n");

    /* ---- Test 1: create / free ---- */
    {
        TextPanel* p = panel_create();
        bool ok = (p != NULL && p->line_count == 0 && p->width == 0 && p->capacity > 0);
        printf("Test 1 - panel_create:\n");
        printf("  not null, count=0, width=0, capacity>0: %s\n\n", ok ? "OK" : "FAIL");
        panel_free(p);
        panel_free(NULL);   /* must not crash */
    }

    /* ---- Test 2: panel_add_line width tracking (ASCII) ---- */
    {
        TextPanel* p = panel_create();
        panel_add_line(p, "hello");       /* 5 cols */
        panel_add_line(p, "world!!");     /* 7 cols */
        panel_add_line(p, "hi");          /* 2 cols */

        bool height_ok = (panel_height(p) == 3);
        bool width_ok  = (panel_width(p)  == 7);
        bool w0_ok     = (p->widths[0] == 5);
        bool w1_ok     = (p->widths[1] == 7);
        bool w2_ok     = (p->widths[2] == 2);

        printf("Test 2 - panel_add_line ASCII width tracking:\n");
        printf("  height == 3: %s\n",   height_ok ? "OK" : "FAIL");
        printf("  width  == 7: %s\n",   width_ok  ? "OK" : "FAIL");
        printf("  widths[0]==5: %s\n",  w0_ok ? "OK" : "FAIL");
        printf("  widths[1]==7: %s\n",  w1_ok ? "OK" : "FAIL");
        printf("  widths[2]==2: %s\n\n",w2_ok ? "OK" : "FAIL");
        panel_free(p);
    }

    /* ---- Test 3: panel_add_line width tracking (Unicode suit symbols) ----
     * ♣ encodes as 3 bytes (E2 99 A3) but is 1 display column.
     * "A♣K♣" is 4 display columns but strlen == 8 bytes.
     */
    {
        TextPanel* p = panel_create();
        panel_add_line(p, "A\xE2\x99\xA3K\xE2\x99\xA3");  /* A♣K♣ — 4 display cols, 8 bytes */
        panel_add_line(p, "ABC");                           /* 3 display cols */

        bool dw_ok     = (p->widths[0] == 4);
        bool width_ok  = (panel_width(p) == 4);

        printf("Test 3 - panel_add_line Unicode width (A♣K♣):\n");
        printf("  display width of A♣K♣ == 4: %s\n",   dw_ok   ? "OK" : "FAIL");
        printf("  panel width == 4:            %s\n\n", width_ok ? "OK" : "FAIL");
        panel_free(p);
    }

    /* ---- Test 4: panel_pad_width ---- */
    {
        TextPanel* p = panel_create();
        panel_add_line(p, "ab");    /* 2 cols */
        panel_add_line(p, "abcde"); /* 5 cols */
        panel_add_line(p, "x");     /* 1 col  */

        panel_pad_width(p, 6);

        /* All lines should now be exactly 6 display columns (6 bytes for ASCII) */
        bool all_6 = true;
        for (int32_t i = 0; i < p->line_count; i++) {
            if ((int32_t)strlen(p->lines[i]) != 6) all_6 = false;
            if (p->widths[i] != 6)                  all_6 = false;
        }
        bool width_ok = (panel_width(p) == 6);

        /* A line already wider than target is untouched */
        TextPanel* q = panel_create();
        panel_add_line(q, "abcdefgh");  /* 8 cols */
        panel_pad_width(q, 4);
        bool wider_ok = (p->widths[0] == 6 && (int32_t)strlen(q->lines[0]) == 8);

        printf("Test 4 - panel_pad_width:\n");
        printf("  all lines padded to 6: %s\n",   all_6    ? "OK" : "FAIL");
        printf("  panel width == 6:      %s\n",   width_ok ? "OK" : "FAIL");
        printf("  wider line untouched:  %s\n\n", wider_ok ? "OK" : "FAIL");
        panel_free(p);
        panel_free(q);
    }

    /* ---- Test 5: panel_pad_height ---- */
    {
        TextPanel* p = panel_create();
        panel_add_line(p, "abc");   /* 3 cols */
        panel_add_line(p, "de");    /* 2 cols */
        panel_pad_width(p, 3);      /* uniform at 3 */
        panel_pad_height(p, 5);

        bool height_ok  = (panel_height(p) == 5);
        bool blank_w_ok = (p->widths[2] == 3 && p->widths[3] == 3 && p->widths[4] == 3);
        bool blank_bytes = ((int32_t)strlen(p->lines[2]) == 3);

        printf("Test 5 - panel_pad_height:\n");
        printf("  height == 5:                  %s\n",   height_ok  ? "OK" : "FAIL");
        printf("  blank lines width == 3:        %s\n",   blank_w_ok ? "OK" : "FAIL");
        printf("  blank line strlen == 3:        %s\n\n", blank_bytes ? "OK" : "FAIL");
        panel_free(p);
    }

    /* ---- Test 6: panel_stack (non-consuming) ---- */
    {
        TextPanel* a = panel_create();
        panel_add_line(a, "top1");
        panel_add_line(a, "top2");

        TextPanel* b = panel_create();
        panel_add_line(b, "bot1");
        panel_add_line(b, "bot2");
        panel_add_line(b, "bot3");

        TextPanel* s = panel_stack(a, b);

        bool height_ok  = (panel_height(s) == 5);
        bool order_ok   = (strcmp(s->lines[0], "top1") == 0 &&
                           strcmp(s->lines[2], "bot1") == 0 &&
                           strcmp(s->lines[4], "bot3") == 0);
        /* Inputs must still be valid */
        bool inputs_ok  = (panel_height(a) == 2 && panel_height(b) == 3);

        printf("Test 6 - panel_stack (non-consuming):\n");
        printf("  height == 5:          %s\n",   height_ok ? "OK" : "FAIL");
        printf("  content order:        %s\n",   order_ok  ? "OK" : "FAIL");
        printf("  inputs still valid:   %s\n\n", inputs_ok ? "OK" : "FAIL");
        panel_free(a);
        panel_free(b);
        panel_free(s);
    }

    /* ---- Test 7: panel_stack_consume ---- */
    {
        TextPanel* a = panel_create();
        panel_add_line(a, "X");
        TextPanel* b = panel_create();
        panel_add_line(b, "Y");

        TextPanel* s = panel_stack_consume(a, b);
        /* a and b are now freed; only s is valid */

        bool height_ok = (panel_height(s) == 2);
        bool x_ok      = (strcmp(s->lines[0], "X") == 0);
        bool y_ok      = (strcmp(s->lines[1], "Y") == 0);

        printf("Test 7 - panel_stack_consume:\n");
        printf("  height == 2:   %s\n",   height_ok ? "OK" : "FAIL");
        printf("  lines[0]=='X': %s\n",   x_ok  ? "OK" : "FAIL");
        printf("  lines[1]=='Y': %s\n\n", y_ok  ? "OK" : "FAIL");
        panel_free(s);
    }

    /* ---- Test 8: panel_join — column alignment ----
     * Left panel has lines of varying natural width; panel_join must pad
     * them all to left->width on-the-fly so the right column aligns.
     *
     * left->width == 5 after adding "ab" (2) and "abcde" (5).
     * With gap=2 every joined row must start with exactly 5+2=7 columns
     * before the right content begins.
     */
    {
        TextPanel* left = panel_create();
        panel_add_line(left, "ab");     /* 2 cols — will be padded to 5 on join */
        panel_add_line(left, "abcde"); /* 5 cols */

        TextPanel* right = panel_create();
        panel_add_line(right, "R1");
        panel_add_line(right, "R2");

        TextPanel* j = panel_join(left, right, 2);

        /* Row 0: "ab" (2) + 3 pad + 2 gap + "R1" = "ab   " + "  " + "R1"
         *         byte offset of 'R' should be 7                           */
        bool align_ok  = (j->lines[0][7] == 'R' && j->lines[1][7] == 'R');
        bool height_ok = (panel_height(j) == 2);
        /* left->width == 5, gap == 2, right max = 2  →  total width == 9  */
        bool width_ok  = (panel_width(j) == 9);

        printf("Test 8 - panel_join column alignment:\n");
        printf("  height == 2:                  %s\n",   height_ok ? "OK" : "FAIL");
        printf("  right col starts at byte 7:   %s\n",   align_ok  ? "OK" : "FAIL");
        printf("  joined width == 9:            %s\n\n", width_ok  ? "OK" : "FAIL");
        panel_free(left);
        panel_free(right);
        panel_free(j);
    }

    /* ---- Test 9: panel_join — height mismatch ---- */
    {
        TextPanel* left = panel_create();
        panel_add_line(left, "L1");
        panel_add_line(left, "L2");
        panel_add_line(left, "L3");

        TextPanel* right = panel_create();
        panel_add_line(right, "R1");

        TextPanel* j = panel_join(left, right, 1);

        bool height_ok = (panel_height(j) == 3);
        /* Row 1 and 2: right side absent → right part is empty string      */
        /* Left still padded to left->width (2) + gap (1) = offset 3       */
        bool row1_ok   = (j->lines[1][3] == '\0');  /* nothing after gap    */

        printf("Test 9 - panel_join height mismatch (left taller):\n");
        printf("  height == 3:                  %s\n",   height_ok ? "OK" : "FAIL");
        printf("  row 1 right side empty:       %s\n\n", row1_ok   ? "OK" : "FAIL");
        panel_free(left);
        panel_free(right);
        panel_free(j);
    }

    /* ---- Test 10: panel_join_consume — chained pipeline ---- */
    {
        TextPanel* a = panel_create();
        panel_add_line(a, "AA");
        TextPanel* b = panel_create();
        panel_add_line(b, "BB");
        TextPanel* c = panel_create();
        panel_add_line(c, "CC");

        /* Chain: join a+b, then join result+c. Intermediates freed inside. */
        TextPanel* result = panel_join_consume(
            panel_join_consume(a, b, 1),
            c, 1
        );

        /* "AA" + " " + "BB" + " " + "CC" = "AA BB CC" (8 chars) */
        bool width_ok   = (panel_width(result) == 8);
        bool content_ok = (strcmp(result->lines[0], "AA BB CC") == 0);

        printf("Test 10 - panel_join_consume chained pipeline:\n");
        printf("  width == 8:              %s\n",   width_ok   ? "OK" : "FAIL");
        printf("  content == 'AA BB CC':   %s\n\n", content_ok ? "OK" : "FAIL");
        panel_free(result);
    }

    /* ---- Test 11: panel_print via open_memstream capture ---- */
    {
        TextPanel* p = panel_create();
        panel_add_line(p, "line one");
        panel_add_line(p, "line two");

        char*  buf = NULL;
        size_t len = 0;
        FILE*  mem = open_memstream(&buf, &len);
        Renderer r = render_default();
        render_set_sink(&r, mem);
        panel_print(p, &r);
        fclose(mem);

        /* Expected: "line one\nline two\n" */
        bool ok = (strcmp(buf, "line one\nline two\n") == 0);
        printf("Test 11 - panel_print output:\n");
        printf("  content matches: %s\n\n", ok ? "OK" : "FAIL");
        free(buf);
        panel_free(p);
    }

    /* ---- Test 12: open_memstream capture into panel ---- */
    {
        /* Capture a views_htr_grid call into a panel */
        HandTypeRange full = htr_full();

        char*  buf = NULL;
        size_t len = 0;
        FILE*  mem = open_memstream(&buf, &len);
        Renderer r = render_default();
        render_set_sink(&r, mem);
        views_htr_grid(&r, &full);
        fclose(mem);

        TextPanel* p = panel_create();
        char* cursor = buf;
        char* nl;
        while ((nl = strchr(cursor, '\n')) != NULL) {
            *nl = '\0';
            panel_add_line(p, cursor);
            cursor = nl + 1;
        }
        if (*cursor) panel_add_line(p, cursor);
        free(buf);

        /* htr_grid emits 13 rows + 1 trailing blank line */
        bool height_ok = (panel_height(p) == 14);
        bool width_ok  = (panel_width(p)  >  0);

        printf("Test 12 - capture views_htr_grid into panel:\n");
        printf("  height == 14 (13 rows + blank): %s\n", height_ok ? "OK" : "FAIL");
        printf("  width > 0:    %s\n\n", width_ok  ? "OK" : "FAIL");

        /* Visual: print captured panel to stdout */
        Renderer out = render_default();
        panel_print(p, &out);

        panel_free(p);
    }

    /* ---- Test 13: panel_print of a joined panel ----
     * Verifies that horizontal composition is preserved faithfully in output.
     * Left "foo"/"bar" (3 cols) joined with right "X"/"Y" (1 col) at gap 2
     * must produce "foo  X" and "bar  Y" as the printed lines.
     */
    {
        TextPanel* left = panel_create();
        panel_add_line(left, "foo");
        panel_add_line(left, "bar");

        TextPanel* right = panel_create();
        panel_add_line(right, "X");
        panel_add_line(right, "Y");

        TextPanel* j = panel_join_consume(left, right, 2);

        char*  buf = NULL;
        size_t len = 0;
        FILE*  mem = open_memstream(&buf, &len);
        Renderer r = render_default();
        render_set_sink(&r, mem);
        panel_print(j, &r);
        fclose(mem);

        bool row0_ok = (strncmp(buf, "foo  X\n", 7) == 0);
        bool row1_ok = (strstr(buf, "bar  Y\n") != NULL);

        printf("Test 13 - panel_print of joined panel:\n");
        printf("  row 0 == 'foo  X': %s\n",   row0_ok ? "OK" : "FAIL");
        printf("  row 1 == 'bar  Y': %s\n\n", row1_ok ? "OK" : "FAIL");
        free(buf);
        panel_free(j);
    }

    /* ---- Test 14: panel_print of a stacked panel ---- */
    {
        TextPanel* top = panel_create();
        panel_add_line(top, "AAA");

        TextPanel* bot = panel_create();
        panel_add_line(bot, "BBB");
        panel_add_line(bot, "CCC");

        TextPanel* s = panel_stack_consume(top, bot);

        char*  buf = NULL;
        size_t len = 0;
        FILE*  mem = open_memstream(&buf, &len);
        Renderer r = render_default();
        render_set_sink(&r, mem);
        panel_print(s, &r);
        fclose(mem);

        bool ok = (strcmp(buf, "AAA\nBBB\nCCC\n") == 0);

        printf("Test 14 - panel_print of stacked panel:\n");
        printf("  content == 'AAA\\nBBB\\nCCC\\n': %s\n\n", ok ? "OK" : "FAIL");
        free(buf);
        panel_free(s);
    }

    /* ---- Test 15: panel_print of empty panel does not crash ---- */
    {
        TextPanel* p = panel_create();

        char*  buf = NULL;
        size_t len = 0;
        FILE*  mem = open_memstream(&buf, &len);
        Renderer r = render_default();
        render_set_sink(&r, mem);
        panel_print(p, &r);
        fclose(mem);

        bool ok = (len == 0);

        printf("Test 15 - panel_print of empty panel:\n");
        printf("  no output, no crash: %s\n\n", ok ? "OK" : "FAIL");
        free(buf);
        panel_free(p);
    }

}
