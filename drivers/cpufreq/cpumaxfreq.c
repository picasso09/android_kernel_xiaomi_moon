#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/cpufreq.h>
#include <linux/seq_file.h>

static struct proc_dir_entry *entry;

static unsigned int cpumaxfreq = 0;

static int cpumaxfreq_read(struct seq_file *m, void *v)
{
	int ret = 0;
	ret = cpumaxfreq/1000;
	seq_printf(m, "%u.%02u\n", ret/1000, ret%1000/10);
	return 0;
}

static int cpumaxfreq_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cpumaxfreq_read, NULL);
}

static const struct proc_ops cpumaxfreq_fops = {
	.proc_open	= cpumaxfreq_proc_open,
	.proc_read	= seq_read,
	.proc_lseek	= seq_lseek,
	.proc_release	= single_release,
};

static int __init cpumaxfreq_init(void)
{
	int cpu;
	struct cpufreq_policy policy;
	entry = proc_create("cpumaxfreq", 0444 /* read only*/,
		NULL /* parent dir */, &cpumaxfreq_fops);
	printk(KERN_INFO "%s,entry: %u kHz\n", __func__, cpumaxfreq);
	for_each_possible_cpu(cpu) {
		if (cpufreq_get_policy(&policy, cpu))
			continue;
		if (policy.cpuinfo.max_freq > cpumaxfreq)
			cpumaxfreq = policy.cpuinfo.max_freq;
	}
	printk(KERN_INFO "%s,end: %u kHz\n", __func__, cpumaxfreq);
	return 0;
}
late_initcall(cpumaxfreq_init);

static void __exit cpumaxfreq_exit(void)
{
	proc_remove(entry);
}
module_exit(cpumaxfreq_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Return the max freqency supported by any core of the processor");