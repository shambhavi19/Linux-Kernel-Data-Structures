#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/hashtable.h>
#include <linux/radix-tree.h>
#include <linux/xarray.h>
#include <linux/rbtree.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/types.h>

static char *int_str;
static int count = 0;
static int *array;
static int *hash_array;
static int *radix_array;
static int *xarray_array;
//static int *rbtree_array;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHAMBHAVI KUTHE");
MODULE_DESCRIPTION("LKP Project 2");

module_param(int_str, charp, S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(int_str, "A comma-separated list of integers");

static LIST_HEAD(mylist);
struct entry {
	int val;
	struct list_head list;
};

struct hash_entry{
	int data;
	struct hlist_node hlist;
};

//struct rb_entry{
//	int data;
//	struct rb_node node;
//};

DEFINE_HASHTABLE(myhash, 3);
DEFINE_XARRAY(myxarray);
RADIX_TREE(myradix, GFP_KERNEL);
//static struct rb_root myrbtree = RB_ROOT;

static int proj2_show(struct seq_file *m, void *v){

		seq_printf(m, "Linked list: %d", array[0]);
		for(int i =1; i<count; i++){
			seq_printf(m, ", %d", array[i]);
		}

		seq_printf(m, "\nHash table: %d", hash_array[0]);
		for(int j = 1; j<count; j++){
			seq_printf(m, ", %d", hash_array[j]);
		}

//		seq_printf(m, "\nRed-black tree: %d", rbtree_array[0]);
//		for(int n = 1; n<count; n++){
//			seq_printf(m, ", %d", rbtree_array[n]);
//		}

		seq_printf(m, "\nRadix tree: %d", radix_array[1]);
		for(int k = 2; k<=count; k++){
			seq_printf(m, ", %d", radix_array[k]);
		}

		seq_printf(m, "\nXArray: %d", xarray_array[1]);
		for(int l = 2; l<=count; l++){
			seq_printf(m, ", %d", xarray_array[l]);
		}

		seq_printf(m, "\n");
		return 0;

}

static int proj2_open(struct inode *inode, struct file *file){
return single_open(file, proj2_show, NULL);
}

static const struct proc_ops proj2_fops = {
	.proc_open = proj2_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

//bool less(struct rb_node* curr, const struct rb_node* par)
//{
//	if((rb_entry(curr, struct rb_entry, node)->data) > (rb_entry(par, struct rb_entry, node)->data))
//	{
//	    	return false;
  //  	}
//	else
//	{
//		return true;
//	}
//}

//static int store_rbtree(int val)
//{
//	struct rb_entry *rb;
//	rb = (struct rb_entry*)kmalloc(sizeof(*rb), GFP_KERNEL);
//
//	if(!rb)
//	{
//		printk(KERN_INFO "RBTREE MEMORY ALLOCATION FAILURE. \n");
//		return -ENOMEM;
//	}
//	else
//	{
//		rb->data = val;
//		rb_add(&(rb->node), &myrbtree, less);
//		return 0;
//	}
//}

static int store_radix(int val)
{
	int *r;
	r = (int*)kmalloc(sizeof(*r), GFP_KERNEL);

	if(!r)
	{
		printk(KERN_INFO "RADIX MEMORY ALLOCATION FAILURE.\n");
		return -ENOMEM;
	}
	else
	{
		*r = val;
		radix_tree_preload(GFP_KERNEL);
		radix_tree_insert(&myradix, val, r);
		radix_tree_preload_end();
		return 0;
	}
}
static int store_xarray(int val)
{
	int *x;
	x = (int*)kmalloc(sizeof(*x), GFP_KERNEL);

	if(!x)
	{
		printk(KERN_INFO "XARRAY MEMORY ALLOCATION FAILURE. \n");
		return -ENOMEM;
	}
	else
	{
		*x = val;
		xa_store(&myxarray, val, (void *)x, GFP_KERNEL);
		return 0;
	}
}
static int store_hash(int val)
{
	struct hash_entry *h;
	h = (struct hash_entry*)kmalloc(sizeof(struct hash_entry*),GFP_KERNEL);

	if(!h)
	{
		printk(KERN_INFO "HASH MEMORY ALLOCATION FAILURE.\n");
		return -ENOMEM;
	}
	else
	{
		h->data = val;
		hash_add(myhash, &h->hlist, h->data);
		return 0;
	}
}
//static void test_rbtree(void)
//{
//	struct rb_node *node;
//	int t = 0;
//	printk(KERN_INFO "Red-black tree: ");
//	for(node = rb_first(&myrbtree); node; node = rb_next(node))
//	{
//		if(node)
//		{
//			printk(KERN_CONT "%d, ", rb_entry(node, struct rb_entry, node)->data);
//			rbtree_array[t] = rb_entry(node, struct rb_entry, node)->data;
//			t++;
//		}
//	}
//	printk(KERN_CONT "\n");

//}

static void test_radix_tree(void)
{

	int *temp;
	printk(KERN_CONT "Radix tree: ");
	for(int r = 0; r<=count; r++)
	{
		temp = (int*)radix_tree_lookup(&myradix, r);
		if(temp)
		{
			radix_array[r] = *temp;
			printk(KERN_CONT "%d, ", *temp);
		}
	}
	printk(KERN_CONT "\n");
}
//
static void test_xarray(void)
{
    int x;
	int *temp;
	printk(KERN_CONT "Xarray: ");
	for(x = 0; x<=count; x++)
	{
		temp = (int*)xa_load(&myxarray, x);
		if(temp)
		{
			xarray_array[x] = *temp;
			printk(KERN_CONT "%d, ", (*temp));
		}
	}
	printk(KERN_CONT "\n");
}

static void test_hash_table(void)
{
	int q = 0;
	int bkt;
	struct hash_entry *pos;
	printk(KERN_CONT "Hash table: ");
	hash_for_each(myhash, bkt, pos, hlist)
	{
        printk(KERN_CONT "%d, ", pos->data);
		hash_array[q] = pos->data;
		q++;
	}
	printk(KERN_CONT "\n");
}

static int store_value(int val)
{

	struct entry *e;
	e = (struct entry*)kmalloc(sizeof(struct entry*),GFP_KERNEL);

	if(!e)
	{
		printk(KERN_INFO "Memory allocation failure. \n");
		return -ENOMEM;
	}
	else
	{
		e->val = val;
		INIT_LIST_HEAD(&e->list);
		list_add_tail(&e->list, &mylist);
		return 0;
	}
}

static void test_linked_list(void)
{
	int p = 0;
	printk(KERN_CONT "Linked list: ");
	struct entry *position;
	struct list_head *lh;
//	list_for_each_entry(position, &mylist, list)
    	list_for_each(lh, &mylist)
	{
		position = list_entry(lh,struct entry, list);
		printk(KERN_CONT "%d, ", position->val);
		array[p] = position->val;
		p++;
	}
	printk(KERN_CONT "\n");
}

static void destroy_radix_tree_and_free(void)
{
	void *temp;
    	for(int k = 0; k<=count; k++)
	{
		temp = radix_tree_delete(&myradix, k);
		if(temp)
		{
		    kfree(temp);
		}
	}
}

//static void destroy_rb_tree_and_free(void)
//{
//	struct rb_node *node;
//	struct rb_entry *temp;
//	for(node=rb_first(&myrbtree);node;node=rb_next(node))
//	{
//		temp = rb_entry(node, struct rb_entry, node);
//		rb_erase(node, &myrbtree);
//		kfree(temp);
//	}
//}

static void destroy_xarray_and_free(void)
{
	for(int x = 0; x<=count; x++)
	{
		void *temp;
		temp = xa_load(&myxarray, x);
		if(temp)
        	{
			kfree(temp);
        	}
	}
	xa_destroy(&myxarray);
}

static void destroy_hash_table_and_free(void)
{
	int hbkt;
	struct hash_entry *hcur;
	struct hlist_node *htemp;
	hash_for_each_safe(myhash, hbkt, htemp, hcur, hlist)
	{
		hash_del(&hcur->hlist);
		kfree(hcur);
	}
}

static void destroy_linked_list_and_free(void)
{
	struct entry *cursor;
	struct entry *temp;
	list_for_each_entry_safe(cursor, temp, &mylist, list)
	{
		list_del(&cursor->list);
		kfree(cursor);
	}
}

static int parse_params(void)
{
	int val, err = 0;
	char *p, *orig, *params;

	params = kstrdup(int_str, GFP_KERNEL);
	if (!params)
		return -ENOMEM;
	orig = params;

	while ((p = strsep(&params, ",")) != NULL) {
		if (!*p)
			continue;
		
		err = kstrtoint(p, 0, &val);
		if (err)
			break;


		err = store_value(val);
        if (err)
            break;
		err = store_hash(val);
        if (err)
            break;
		err = store_radix(val);
        if (err)
            break;
		err = store_xarray(val);
        if (err)
            break;
  //      err = store_rbtree(val);

        count++;

        if (err)
            break;

	}

	array = (int*)kmalloc(sizeof(int)*count,GFP_KERNEL);
	hash_array = (int*)kmalloc(sizeof(int)*count, GFP_KERNEL);
 	radix_array = (int*)kmalloc(sizeof(int)*count, GFP_KERNEL);
	xarray_array = (int*)kmalloc(sizeof(int)*count, GFP_KERNEL);
//	rbtree_array = (int*)kmalloc(sizeof(int)*count, GFP_KERNEL);

	kfree(orig);
	return err;
}

static void run_tests(void)
{
	test_linked_list();
	test_hash_table();
//	test_rbtree();
	test_radix_tree();
	test_xarray();
	proc_create("proj2", 0, NULL, &proj2_fops);
}

static void cleanup(void)
{
	printk(KERN_INFO "\nCleaning up...\n");

	destroy_linked_list_and_free();
	destroy_hash_table_and_free();
	destroy_radix_tree_and_free();
	destroy_xarray_and_free();
//	printk(KERN_INFO "Cleaned Xarray\n");
//	destroy_rb_tree_and_free();
//	printk(KERN_INFO "Cleaned RBtress\n");
	
}

static int __init ex3_init(void)
{

	hash_init(myhash);
    	xa_init(&myxarray);
    	int err = 0;

	if (!int_str) {
		printk(KERN_INFO "Missing \'int_str\' parameter, exiting\n");
		return -1;
	}

	err = parse_params();
	if (err)
		goto out;

	run_tests();
out:
	cleanup();
	return err;
}

static void __exit ex3_exit(void)
{
	remove_proc_entry("proj2", NULL);
	kfree(array);
	kfree(hash_array);
	kfree(xarray_array);
	kfree(radix_array);
//	kfree(rbtree_array);
	return;
}

module_init(ex3_init);

module_exit(ex3_exit);
