bool qtree_alloc(qtree ** qt, int depth)
{
    if (*qt == NULL) {
        if (! (*qt = (qtree*)calloc(1, sizeof(qtree)))) {
            return false;
        }

        if (! ((*qt)->map = (qnode**)calloc(depth+1, sizeof(qnode*)))) {
            return false;
        }

        int length =  1;
        int i;
        for (i = 0; i < depth + 1; i++) {
            if (! ((*qt)->map[i] = (qnode*)calloc(length, sizeof(qnode)))) {
                return false;
            }
        }
    }
    
    int length = 1;
    for (int i = 0; i < depth; i++) {
        for (int j = 0; j < length; j++) {
            (*qt)->map[i][j].next = &(*qt)->map[i+1][4*j];
        }
        length *= 4;
    }
    
    return true;
}
